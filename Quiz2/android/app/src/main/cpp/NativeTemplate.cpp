/**
 * NativeTemplate.cpp – JNI bridge for VertexArrayObject-Modern.
 *   package com.example.vertexarrayobject
 *   class   MainActivity
 */

#define LOG_TAG "VertexArrayObjectNative"
#ifndef PLATFORM_ANDROID
#define PLATFORM_ANDROID
#endif

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "Renderer.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

extern "C" JNIEXPORT jboolean JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeInit(
        JNIEnv* env, jobject, jobject assetManager)
{
    LOGI("nativeInit");
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    if (!mgr) { LOGE("Failed to get AAssetManager"); return JNI_FALSE; }
    Renderer::Instance().setAssetManager(mgr);
    return Renderer::Instance().initializeRenderer() ? JNI_TRUE : JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeResize(
        JNIEnv*, jobject, jint w, jint h)
{ Renderer::Instance().resize(w, h); }

extern "C" JNIEXPORT void JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeRender(JNIEnv*, jobject)
{ Renderer::Instance().render(); }

extern "C" JNIEXPORT void JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeTouchDown(
        JNIEnv*, jobject, jfloat x, jfloat y)
{ Renderer::Instance().TouchEventDown(x, y); }

extern "C" JNIEXPORT void JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeTouchMove(
        JNIEnv*, jobject, jfloat x, jfloat y)
{ Renderer::Instance().TouchEventMove(x, y); }

extern "C" JNIEXPORT void JNICALL
Java_com_example_vertexarrayobject_MainActivity_nativeTouchRelease(
        JNIEnv*, jobject, jfloat x, jfloat y)
{ Renderer::Instance().TouchEventRelease(x, y); }
