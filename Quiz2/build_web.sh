#!/usr/bin/env sh
# Build VertexArrayObject-Modern for WebGL using Emscripten.
# Activate emsdk first: source /path/to/emsdk/emsdk_env.sh
#
# Windowing backend (choose one):
#   GLFW  – default  (-DUSE_GLFW -s USE_GLFW=3)
#   SDL2  – opt-in   (no -DUSE_GLFW, -s USE_SDL=2)
set -e
mkdir -p web

if [ ! -d external/glm ]; then
    git clone --depth 1 https://github.com/g-truc/glm.git external/glm
else
    git -C external/glm pull
fi

# --- GLFW backend (default) -------------------------------------------------
emcc -std=c++17 \
    Scene/main.cpp \
    Scene/Renderer.cpp \
    Scene/Transform.cpp \
    Scene/Fan.cpp \
    Scene/Cube.cpp \
    Scene/Grid.cpp \
    -IScene \
    -Iexternal/glm \
    -DUSE_GLFW \
    -s USE_GLFW=3 \
    -s USE_WEBGL2=1 \
    -s FULL_ES3=1 \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file android/app/src/main/assets/shader@assets/shader \
    -o web/index.html

# --- SDL2 backend (opt-in) --------------------------------------------------
# emcc -std=c++17 \
#     Scene/main.cpp \
#     Scene/Renderer.cpp \
#     Scene/Transform.cpp \
#     Scene/Fan.cpp \
#     Scene/Cube.cpp \
#     Scene/Grid.cpp \
#     -IScene \
#     -Iexternal/glm \
#     -s USE_SDL=2 \
#     -s USE_WEBGL2=1 -s FULL_ES3=1 \
#     -s WASM=1 \
#     -s ALLOW_MEMORY_GROWTH=1 \
#     --preload-file android/app/src/main/assets/shader@assets/shader \
#     -o web/index.html

echo "Build succeeded. cd web && python3 -m http.server 8080"
