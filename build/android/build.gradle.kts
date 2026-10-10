// Top-level build file where you can add configuration options common to all sub-projects/modules.
buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        // com.ydq.android.gradle.native-aar.import plugin
        classpath(libs.android.native.bundle)
    }
}

plugins {
    alias(libs.plugins.android.application) apply false
}