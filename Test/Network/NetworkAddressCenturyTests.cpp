// NetworkAddressCenturyTests.cpp
//
// 50 tests for kai::net::IpAddress / kai::net::MacAddress (Include/KAI/Network/Address.h).
// Unlike most of the other files in Test/Network, these are pure value-type
// unit tests with no sockets, no Node/Agent/Proxy setup, and no timeouts -
// they exercise construction, ToString/FromString, the GetPort/GetAddress
// "host:port" string-splitting helpers, equality, and the Hash()/std::hash
// specializations that let these types be used as unordered_map/unordered_set
// keys. Fast and deterministic, so no port-availability skipping is needed.

#include <gtest/gtest.h>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "KAI/Network/Address.h"

using kai::net::IpAddress;
using kai::net::MacAddress;

// --------------------------------------------------------------------------
// IpAddress: construction / ToString (6)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, DefaultConstructedIpAddressIsEmpty) {
    IpAddress addr;
    EXPECT_EQ(addr.Text(), "");
}

TEST(NetworkAddressCenturyTests, ConstructFromLiteralText) {
    IpAddress addr("10.0.0.1");
    EXPECT_EQ(addr.Text(), "10.0.0.1");
}

TEST(NetworkAddressCenturyTests, ToStringMatchesText) {
    IpAddress addr("192.168.0.42");
    EXPECT_EQ(addr.ToString(), addr.Text());
}

TEST(NetworkAddressCenturyTests, TextMatchesConstructorArgument) {
    std::string original = "172.16.5.5";
    IpAddress addr(original);
    EXPECT_EQ(addr.Text(), original);
}

TEST(NetworkAddressCenturyTests, LocalhostIsCorrectString) {
    EXPECT_EQ(IpAddress::Localhost().ToString(), "127.0.0.1");
}

TEST(NetworkAddressCenturyTests, BroadcastIsCorrectString) {
    EXPECT_EQ(IpAddress::Broadcast().ToString(), "255.255.255.255");
}

// --------------------------------------------------------------------------
// IpAddress: FromString (4)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, FromStringSimple) {
    auto addr = IpAddress::FromString("8.8.8.8");
    EXPECT_EQ(addr.ToString(), "8.8.8.8");
}

TEST(NetworkAddressCenturyTests, FromStringEqualsDirectConstruction) {
    EXPECT_EQ(IpAddress::FromString("1.2.3.4"), IpAddress("1.2.3.4"));
}

TEST(NetworkAddressCenturyTests, FromStringEmpty) {
    auto addr = IpAddress::FromString("");
    EXPECT_EQ(addr.ToString(), "");
}

TEST(NetworkAddressCenturyTests, FromStringWithPortStoresVerbatim) {
    // FromString does no host/port splitting - it stores exactly what's given.
    auto addr = IpAddress::FromString("192.168.1.5:9000");
    EXPECT_EQ(addr.ToString(), "192.168.1.5:9000");
}

// --------------------------------------------------------------------------
// IpAddress::GetPort (10)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, GetPortWithValidPort) {
    EXPECT_EQ(IpAddress::GetPort("127.0.0.1:8080"), 8080);
}

TEST(NetworkAddressCenturyTests, GetPortWithoutColonReturnsDefault) {
    EXPECT_EQ(IpAddress::GetPort("127.0.0.1", 1234), 1234);
}

TEST(NetworkAddressCenturyTests, GetPortDefaultsToZeroWhenUnspecified) {
    EXPECT_EQ(IpAddress::GetPort("no-colon-here"), 0);
}

TEST(NetworkAddressCenturyTests, GetPortEmptyStringReturnsDefault) {
    EXPECT_EQ(IpAddress::GetPort("", 42), 42);
}

TEST(NetworkAddressCenturyTests, GetPortNonNumericPortReturnsDefault) {
    EXPECT_EQ(IpAddress::GetPort("host:abc", 42), 42);
}

TEST(NetworkAddressCenturyTests, GetPortWithZeroPort) {
    EXPECT_EQ(IpAddress::GetPort("host:0", 99), 0);
}

TEST(NetworkAddressCenturyTests, GetPortWithMaxPortNumber) {
    EXPECT_EQ(IpAddress::GetPort("host:65535"), 65535);
}

TEST(NetworkAddressCenturyTests, GetPortWithMultipleColonsUsesFirstSplit) {
    // Splits on the *first* colon, then parses leading digits of the rest.
    EXPECT_EQ(IpAddress::GetPort("host:1234:extra"), 1234);
}

TEST(NetworkAddressCenturyTests, GetPortWithNegativePortString) {
    EXPECT_EQ(IpAddress::GetPort("host:-1"), -1);
}

TEST(NetworkAddressCenturyTests, GetPortWithLeadingZeros) {
    EXPECT_EQ(IpAddress::GetPort("host:00080"), 80);
}

// --------------------------------------------------------------------------
// IpAddress::GetAddress (6)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, GetAddressWithPort) {
    EXPECT_EQ(IpAddress::GetAddress("192.168.1.5:9000"), "192.168.1.5");
}

TEST(NetworkAddressCenturyTests, GetAddressWithoutPort) {
    EXPECT_EQ(IpAddress::GetAddress("192.168.1.5"), "192.168.1.5");
}

TEST(NetworkAddressCenturyTests, GetAddressEmptyString) {
    EXPECT_EQ(IpAddress::GetAddress(""), "");
}

TEST(NetworkAddressCenturyTests, GetAddressOnlyColon) {
    EXPECT_EQ(IpAddress::GetAddress(":1234"), "");
}

TEST(NetworkAddressCenturyTests, GetAddressWithMultipleColonsUsesFirstSplit) {
    EXPECT_EQ(IpAddress::GetAddress("a:b:c"), "a");
}

TEST(NetworkAddressCenturyTests, GetAddressPreservesCase) {
    EXPECT_EQ(IpAddress::GetAddress("MyHost:80"), "MyHost");
}

// --------------------------------------------------------------------------
// IpAddress: equality (6)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, EqualIpAddressesAreEqual) {
    EXPECT_EQ(IpAddress("10.1.1.1"), IpAddress("10.1.1.1"));
}

TEST(NetworkAddressCenturyTests, DifferentIpAddressesAreNotEqual) {
    EXPECT_NE(IpAddress("10.1.1.1"), IpAddress("10.1.1.2"));
}

TEST(NetworkAddressCenturyTests, NotEqualOperatorConsistentWithEqual) {
    IpAddress a("1.1.1.1"), b("1.1.1.1"), c("2.2.2.2");
    EXPECT_FALSE(a != b);
    EXPECT_TRUE(a != c);
}

TEST(NetworkAddressCenturyTests, DefaultConstructedIpAddressesAreEqual) {
    EXPECT_EQ(IpAddress(), IpAddress());
}

TEST(NetworkAddressCenturyTests, LocalhostEqualsLocalhost) {
    EXPECT_EQ(IpAddress::Localhost(), IpAddress::Localhost());
}

TEST(NetworkAddressCenturyTests, ComparisonIsCaseSensitive) {
    EXPECT_NE(IpAddress("Host"), IpAddress("host"));
}

// --------------------------------------------------------------------------
// IpAddress: hashing (6)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, HashIsConsistentForSameAddress) {
    IpAddress a("10.0.0.5"), b("10.0.0.5");
    EXPECT_EQ(a.Hash(), b.Hash());
}

TEST(NetworkAddressCenturyTests, HashDiffersForClearlyDifferentAddresses) {
    IpAddress a("10.0.0.5"), b("192.168.99.99");
    EXPECT_NE(a.Hash(), b.Hash());
}

TEST(NetworkAddressCenturyTests, StdHashSpecializationMatchesMemberHash) {
    IpAddress addr("10.0.0.9");
    std::hash<IpAddress> hasher;
    EXPECT_EQ(hasher(addr), addr.Hash());
}

TEST(NetworkAddressCenturyTests, UsableAsUnorderedMapKey) {
    std::unordered_map<IpAddress, int> ports;
    ports[IpAddress("10.0.0.1")] = 100;
    ports[IpAddress("10.0.0.2")] = 200;
    EXPECT_EQ(ports[IpAddress("10.0.0.1")], 100);
    EXPECT_EQ(ports[IpAddress("10.0.0.2")], 200);
}

TEST(NetworkAddressCenturyTests, UsableInUnorderedSet) {
    std::unordered_set<IpAddress> seen;
    seen.insert(IpAddress("10.0.0.1"));
    seen.insert(IpAddress("10.0.0.1"));  // duplicate
    seen.insert(IpAddress("10.0.0.2"));
    EXPECT_EQ(seen.size(), 2u);
}

TEST(NetworkAddressCenturyTests, HashValueFreeFunctionMatchesMemberHash) {
    IpAddress addr("10.0.0.7");
    EXPECT_EQ(kai::net::HashValue(addr), addr.Hash());
}

// --------------------------------------------------------------------------
// MacAddress (8)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, MacAddressConstructFromText) {
    MacAddress mac("AA:BB:CC:DD:EE:FF");
    EXPECT_EQ(mac.Text(), "AA:BB:CC:DD:EE:FF");
}

TEST(NetworkAddressCenturyTests, MacAddressToStringMatchesText) {
    MacAddress mac("00:11:22:33:44:55");
    EXPECT_EQ(mac.ToString(), mac.Text());
}

TEST(NetworkAddressCenturyTests, MacAddressEquality) {
    EXPECT_EQ(MacAddress("AA:BB:CC:DD:EE:FF"), MacAddress("AA:BB:CC:DD:EE:FF"));
}

TEST(NetworkAddressCenturyTests, MacAddressInequality) {
    EXPECT_NE(MacAddress("AA:BB:CC:DD:EE:FF"), MacAddress("11:22:33:44:55:66"));
}

TEST(NetworkAddressCenturyTests, MacAddressHashConsistent) {
    MacAddress a("DE:AD:BE:EF:00:01"), b("DE:AD:BE:EF:00:01");
    EXPECT_EQ(a.Hash(), b.Hash());
}

TEST(NetworkAddressCenturyTests, MacAddressStdHashSpecializationWorks) {
    MacAddress mac("DE:AD:BE:EF:00:02");
    std::hash<MacAddress> hasher;
    EXPECT_EQ(hasher(mac), mac.Hash());
}

TEST(NetworkAddressCenturyTests, MacAddressUsableAsUnorderedMapKey) {
    std::unordered_map<MacAddress, std::string> names;
    names[MacAddress("AA:AA:AA:AA:AA:AA")] = "nodeA";
    names[MacAddress("BB:BB:BB:BB:BB:BB")] = "nodeB";
    EXPECT_EQ(names[MacAddress("AA:AA:AA:AA:AA:AA")], "nodeA");
    EXPECT_EQ(names[MacAddress("BB:BB:BB:BB:BB:BB")], "nodeB");
}

TEST(NetworkAddressCenturyTests, MacAddressHashValueFreeFunctionMatches) {
    MacAddress mac("CA:FE:BA:BE:00:00");
    EXPECT_EQ(kai::net::HashValue(mac), mac.Hash());
}

// --------------------------------------------------------------------------
// Round trip / combined usage (4)
// --------------------------------------------------------------------------

TEST(NetworkAddressCenturyTests, AddressPortRoundTrip) {
    const std::string combined = "192.168.1.5:9000";
    EXPECT_EQ(IpAddress::GetAddress(combined), "192.168.1.5");
    EXPECT_EQ(IpAddress::GetPort(combined), 9000);
}

TEST(NetworkAddressCenturyTests, RoundTripThroughFromStringAndToString) {
    const std::string original = "203.0.113.7";
    EXPECT_EQ(IpAddress::FromString(original).ToString(), original);
}

TEST(NetworkAddressCenturyTests, MultipleAddressesInVectorPreserveOrder) {
    std::vector<IpAddress> addrs = {IpAddress("1.1.1.1"), IpAddress("2.2.2.2"),
                                    IpAddress("3.3.3.3")};
    ASSERT_EQ(addrs.size(), 3u);
    EXPECT_EQ(addrs[0].ToString(), "1.1.1.1");
    EXPECT_EQ(addrs[1].ToString(), "2.2.2.2");
    EXPECT_EQ(addrs[2].ToString(), "3.3.3.3");
}

TEST(NetworkAddressCenturyTests, DefaultPortFallbackWhenPortMissingFromCombined) {
    const std::string hostOnly = "example.local";
    EXPECT_EQ(IpAddress::GetAddress(hostOnly), "example.local");
    EXPECT_EQ(IpAddress::GetPort(hostOnly, 7777), 7777);
}
