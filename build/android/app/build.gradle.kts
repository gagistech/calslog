plugins {
    alias(libs.plugins.android.application)
}

apply(plugin = "com.ydq.android.gradle.native-aar.import") // must go after android gradle plugin

android {
    namespace = "com.gagistech.app.calslog"
    compileSdk = 37

    defaultConfig {
        applicationId = "com.gagistech.app.calslog"
        minSdk = 24
        targetSdk = 37
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"

        externalNativeBuild {
            cmake {
                targets("ruisapp_application")

                arguments("-DANDROID_STL=c++_shared", "-DANDROID_TOOLCHAIN=clang")
            }
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = true
        }
    }
    // Encapsulates your external native build configurations.
    externalNativeBuild {
        // Encapsulates your CMake build configurations.
        cmake {
            // Provides a relative path to your CMake build script.
            path = file("CMakeLists.txt")
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
}

// Copy the app resources from the repo's res/ directory into the APK assets
// (the ruisapp glue loads them from the asset manager at 'res/').
val copyRes by tasks.registering(Copy::class) {
    from(file("../../../res"))
    into(file("src/main/assets/res"))
    duplicatesStrategy = DuplicatesStrategy.INCLUDE
    outputs.upToDateWhen { false }
}

tasks.named("preBuild") { dependsOn(copyRes) }

dependencies {
    implementation(libs.ruisapp)

    // TODO: are all these needed?
    implementation(libs.appcompat)
    implementation(libs.material)
    testImplementation(libs.junit)
    androidTestImplementation(libs.espresso.core)
    androidTestImplementation(libs.ext.junit)
}