#include "ExecutorWindow.h"

// httplib.h is vendored under Ext/cpp-httplib (already used elsewhere in
// this project's ecosystem, e.g. CppLocalLlmCodeAssist's own chat server -
// this file is the *client* side, talking to that server). Header-only,
// so this is the only translation unit that needs to see it. No
// CPPHTTPLIB_OPENSSL_SUPPORT is defined anywhere in this project, so this
// only ever speaks plain http:// - fine, since it only ever talks to a
// local cppcoder instance on 127.0.0.1.
#include <httplib.h>

#include <cstdio>
#include <sstream>

KAI_BEGIN

namespace {

// Minimal hand-rolled JSON string-field extractor - pulling in a real JSON
// library for one field felt like overkill given the server's response
// shape is fixed and well-known (it's a pass-through of Ollama's own ndjson
// chat format: {"model":..,"message":{"role":"assistant","content":".."},
// "done":false} per line, or {"error":".."} on failure). Finds `"key":"`
// and reads an escaped JSON string out to the closing quote.
std::string ExtractJsonStringField(const std::string& json,
                                   const std::string& key) {
    std::string needle = "\"" + key + "\":\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return {};
    pos += needle.size();

    std::string out;
    while (pos < json.size() && json[pos] != '"') {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            char next = json[pos + 1];
            switch (next) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case '"': out += '"'; break;
                case '\\': out += '\\'; break;
                case '/': out += '/'; break;
                default: out += next; break;  // \uXXXX etc left as-is
            }
            pos += 2;
        } else {
            out += json[pos];
            pos += 1;
        }
    }
    return out;
}

// Escapes a string for embedding as a JSON string value when building the
// request body (same reasoning as above - fixed, simple shape, not worth a
// JSON library dependency just to serialize {"role":..,"content":..}).
std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += static_cast<char>(c);
                }
        }
    }
    return out;
}

std::string BuildChatRequestBody(const std::vector<ChatMessage>& history,
                                 const std::string& model) {
    std::ostringstream body;
    body << "{";
    if (!model.empty()) {
        body << "\"model\":\"" << JsonEscape(model) << "\",";
    }
    body << "\"messages\":[";
    for (size_t i = 0; i < history.size(); ++i) {
        if (i) body << ",";
        body << "{\"role\":\"" << JsonEscape(history[i].role)
             << "\",\"content\":\"" << JsonEscape(history[i].content)
             << "\"}";
    }
    body << "]}";
    return body.str();
}

}  // namespace

void ExecutorWindow::SendAssistantMessage(const std::string& text) {
    if (text.empty() || AssistantBusy.load()) return;

    std::vector<ChatMessage> historySnapshot;
    {
        std::lock_guard<std::mutex> lock(AssistantMutex);
        AssistantHistory.push_back(ChatMessage{"user", text});
        AssistantStreamBuffer.clear();
        AssistantError.clear();
        historySnapshot = AssistantHistory;
    }
    AssistantBusy = true;

    const std::string model = AssistantModel;
    const std::string host = AssistantHost;
    const int port = AssistantPort;

    // Fire-and-forget: SendAssistantMessage() refuses to start a second
    // request while AssistantBusy is true, so there's never more than one
    // of these in flight, and detaching (rather than keeping a joinable
    // std::thread member) avoids needing a destructor here just to join it
    // on shutdown - the one edge case this trades away is closing the app
    // mid-request, an acceptable risk for a dev-tool panel.
    std::thread([this, historySnapshot, model, host, port]() {
        httplib::Client cli(host, port);
        cli.set_connection_timeout(5, 0);
        cli.set_read_timeout(180, 0);  // local LLM inference can be slow

        const std::string body = BuildChatRequestBody(historySnapshot, model);

        std::string lineBuffer;  // ndjson chunks don't align with '\n's
        // The (path, body, content_type, callback) overload's 4th parameter
        // is actually an UploadProgress callback (for the request body going
        // *up*), not a receiver for the response coming *down* - streaming
        // the response needs the ContentReceiver overload instead, which
        // requires an explicit Headers argument ahead of the body.
        auto res = cli.Post(
            "/api/chat", httplib::Headers(), body, "application/json",
            [this, &lineBuffer](const char* data, size_t len) {
                lineBuffer.append(data, len);
                size_t nl;
                while ((nl = lineBuffer.find('\n')) != std::string::npos) {
                    std::string line = lineBuffer.substr(0, nl);
                    lineBuffer.erase(0, nl + 1);
                    if (line.empty()) continue;

                    std::string err = ExtractJsonStringField(line, "error");
                    std::string content =
                        ExtractJsonStringField(line, "content");

                    std::lock_guard<std::mutex> lock(AssistantMutex);
                    if (!err.empty()) {
                        AssistantError = err;
                    } else if (!content.empty()) {
                        AssistantStreamBuffer += content;
                    }
                }
                return true;  // keep receiving
            });

        std::lock_guard<std::mutex> lock(AssistantMutex);
        if (!res) {
            if (AssistantError.empty()) {
                AssistantError =
                    "Couldn't reach " + host + ":" + std::to_string(port) +
                    " - is `cppcoder --serve` running?";
            }
        } else if (res->status != 200 && AssistantError.empty()) {
            AssistantError = "HTTP " + std::to_string(res->status);
        }

        if (!AssistantStreamBuffer.empty()) {
            AssistantHistory.push_back(
                ChatMessage{"assistant", AssistantStreamBuffer});
        }
        AssistantBusy = false;
    }).detach();
}

void ExecutorWindow::DrawAssistantContent() {
    ImGui::TextColored(
        ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
        "Talks to a local `cppcoder --serve` (CppLocalLlmCodeAssist) chat "
        "server over HTTP - start that separately first.");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%s:%d]",
                       AssistantHost.c_str(), AssistantPort);
    ImGui::Separator();

    // Snapshot everything AssistantMutex guards once per frame, rather than
    // holding the lock across the ImGui calls below - the background
    // request thread only ever needs brief critical sections to append a
    // chunk, so a frame's worth of ImGui rendering never blocks it either.
    std::vector<ChatMessage> historyCopy;
    std::string streamCopy;
    std::string errorCopy;
    {
        std::lock_guard<std::mutex> lock(AssistantMutex);
        historyCopy = AssistantHistory;
        streamCopy = AssistantStreamBuffer;
        errorCopy = AssistantError;
    }
    const bool busy = AssistantBusy.load();

    ImGui::BeginChild("AssistantScroll", ImVec2(0, -60), true);
    for (const auto& msg : historyCopy) {
        const bool isUser = (msg.role == "user");
        ImGui::PushStyleColor(ImGuiCol_Text,
                              isUser ? kPromptColor
                                     : ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
        ImGui::TextWrapped("%s: %s", msg.role.c_str(), msg.content.c_str());
        ImGui::PopStyleColor();
        ImGui::Separator();
    }
    if (busy) {
        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.85f, 1.0f),
                           "assistant: %s", streamCopy.c_str());
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "(thinking...)");
        ImGui::SetScrollHereY(1.0f);
    }
    if (!errorCopy.empty()) {
        ImGui::TextColored(kErrorColor, "[Error] %s", errorCopy.c_str());
    }
    ImGui::EndChild();

    ImGui::Separator();
    bool sendNow = false;
    // This ImGui version (1.67) has no SetNextItemWidth() (added later) -
    // PushItemWidth()/PopItemWidth() around the one widget is the
    // equivalent here.
    ImGui::PushItemWidth(-70.0f);
    if (ImGui::InputText("##AssistantInput", AssistantInputBuf,
                         sizeof(AssistantInputBuf),
                         ImGuiInputTextFlags_EnterReturnsTrue)) {
        sendNow = true;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();
    // This ImGui version (1.67) has no BeginDisabled()/EndDisabled(), so the
    // Send button stays visually normal while busy - SendAssistantMessage()
    // itself already refuses to start a second overlapping request, so
    // there's no functional issue, just a missed "greyed out" polish cue.
    if (ImGui::Button("Send") ) {
        sendNow = true;
    }

    if (sendNow && !busy && AssistantInputBuf[0]) {
        // Deliberately not touching FocusInputNextFrame here - that flag is
        // the Pi/Rho tabs' own "reclaim keyboard focus" mechanism (see
        // DrawConsoleContent()) and setting it from here would steal focus
        // there the next time the user switches to one of those tabs.
        SendAssistantMessage(AssistantInputBuf);
        AssistantInputBuf[0] = '\0';
    }
}

KAI_END
