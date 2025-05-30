# ImGui Multiple Installation Issue

## Current Situation:
1. **Ext/imgui/** - The actual ImGui source (used by ImGui library build)
2. **Include/KAI/ImGui/** - Copy of v1.67 WIP (used by Window app)
3. **Include/KAI/Imgui/** - Copy of v1.50 WIP (referenced in Window CMakeLists but uses ImGui)

## Problems:
- Multiple versions cause confusion
- Window app uses copies instead of the actual source
- Case sensitivity issue (ImGui vs Imgui)

## Recommendation:
- Remove both Include/KAI/ImGui and Include/KAI/Imgui directories
- Update Window app to use Ext/imgui directly
- This ensures single source of truth for ImGui