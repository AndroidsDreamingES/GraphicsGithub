@echo off
REM Build VertexArrayObject-Modern for WebGL (Emscripten/Windows)
REM Prerequisite: activate emsdk before running.
REM
REM Windowing backend (choose one):
REM   GLFW  – default, recommended  (-DUSE_GLFW  -s USE_GLFW=3)
REM   SDL2  – legacy fallback        (no -DUSE_GLFW, -s USE_SDL=2)

IF NOT EXIST web MKDIR web

IF NOT EXIST external\glm (
     git  clone --depth 1 https://github.com//g-truc/glm.git external/glm   
) else (
     git -C external\glm pull
)

REM --- GLFW backend (default) -----------------------------------------------
emcc -std=c++17 ^
     Scene/main.cpp ^
     Scene/Renderer.cpp ^
     Scene/Transform.cpp ^
     Scene/Cube.cpp ^
     Scene/Grid.cpp ^
     Scene/Fan.cpp ^
     -IScene ^
     -Iexternal/glm ^
     -DUSE_GLFW ^
     -s USE_GLFW=3 ^
     -s USE_WEBGL2=1 ^
     -s FULL_ES3=1 ^
     -s WASM=1 ^
     -s ALLOW_MEMORY_GROWTH=1 ^
     --preload-file android/app/src/main/assets/shader@assets/shader ^
     -o web/index.html

REM --- SDL2 backend (opt-in) ------------------------------------------------
REM emcc -std=c++17 ^
REM      Scene/main.cpp ^
REM      Scene/Renderer.cpp ^
REM      Scene/Transform.cpp ^
REM      Scene/Cube.cpp ^
REM      Scene/Grid.cpp ^
REM      Scene/Fan.cpp ^
REM      -IScene ^
REM      -Iexternal/glm ^
REM      -s USE_SDL=2 ^
REM      -s USE_WEBGL2=1 ^
REM      -s FULL_ES3=1 ^
REM      -s WASM=1 ^
REM      -s ALLOW_MEMORY_GROWTH=1 ^
REM      --preload-file android/app/src/main/assets/shader@assets/shader ^
REM      -o web/index.html

IF %ERRORLEVEL% NEQ 0 ( echo Build FAILED. & pause & exit /b 1 )
echo Build succeeded.  cd web ^&^& python -m http.server 8080
pause
