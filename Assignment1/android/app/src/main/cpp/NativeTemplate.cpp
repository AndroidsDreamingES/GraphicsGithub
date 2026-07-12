/**
 * NativeTemplate.cpp
 *
 * Thin JNI bridge between the Java/Android layer and the shared Triangle renderer.
 * All GL logic lives in Scene/Triangle.h + Triangle.cpp.
 *
 * Native method signatures match MainActivity.java:
 *   package com.example.glpiframeworkintro
 *   class   MainActivity
 */

//#define LOG_TAG "GLPIFrameworkIntroNative"
// hi i think you should always have this at the top of yo files
#include "Platform.h"

#include <jni.h>
//#include "Triangle.h"   // brings in Platform.h → GLES3/gl3.h + LOGI/LOGE
// you should wrap this android-only header under some #ifdef checks
#ifdef PLATFORM_ANDROID
#include <android/asset_manager_jni.h>
#endif

#include "Renderer.h"
#include "ShaderHelper.h"

//static Triangle g_triangle;

// ---------------------------------------------------------------------------
// JNI entry points
// ---------------------------------------------------------------------------

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeInit(
        JNIEnv* env, jobject /*thiz*/, jobject assetManager)
{
   //LOGI("nativeInit called");
   //Renderer::Instance().InitModel();
//    return JNI_TRUE;

    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().InitializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeResize(
        JNIEnv* /*env*/, jobject /*thiz*/, jint width, jint height)
{
    Renderer::Instance().Resize(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_helloworldandroid_MainActivity_nativeRender(
        JNIEnv* /*env*/, jobject /*thiz*/)
{
    Renderer::Instance().Render();
}
