plugins {
    alias(libs.plugins.android.application)
}

apply(plugin = "com.ydq.android.gradle.native-aar.import") // must go after android gradle plugin

android {
    namespace = "com.gagistech.app.calslog"
    // NOTE: use an integer compile SDK (36), not 37.
    // API 37 is a *floating* level: the SDK installs it as platform dir 'android-37.0'
    // (source.properties ApiLevel=37.0), but AGP computes the target hash from the
    // integer 37 as 'android-37', which does not match that dir. On CI the platform is
    // auto-installed during the build, so the hash lookup 'android-37' fails with
    // "Failed to find target with hash string 'android-37'". 36 is a plain integer
    // (dir 'android-36' == hash 'android-36') and is within AGP 8.13.2's tested range.
    compileSdk = 36

    defaultConfig {
        applicationId = "com.gagistech.app.calslog"
        minSdk = 24
        targetSdk = 36
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