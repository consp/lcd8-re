

plugins {
    alias(libs.plugins.android.application)
    id("com.google.android.libraries.mapsplatform.secrets-gradle-plugin")
    alias(libs.plugins.kotlin.android)
    id("kotlin-parcelize")
    kotlin("plugin.serialization") version "2.2.0"
}

android {
    namespace = "com.example.bikenavunit"
    compileSdk {
        version = release(36)
    }

    defaultConfig {
        applicationId = "com.example.bikenavunit"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        isCoreLibraryDesugaringEnabled = true

        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    kotlinOptions {
        jvmTarget = "1.8"
    }
}

dependencies {
    implementation(libs.appcompat)
    implementation(libs.material)
    implementation(libs.activity)
    implementation(libs.constraintlayout)
    implementation(libs.core.ktx)
    implementation(libs.navigation.runtime)
    testImplementation(libs.junit)
    androidTestImplementation(libs.ext.junit)
    androidTestImplementation(libs.espresso.core)
    implementation(libs.navigation)
    coreLibraryDesugaring(libs.desugar.jdk.libs)
    api(libs.navigation)
    implementation(libs.kotlin.android.extensions.runtime)
    implementation(libs.kotlinx.serialization.json)
    implementation(libs.kotlinx.serialization.core)
    implementation(libs.kotlinx.serialization.cbor)
}

secrets {
    // To add your Maps API key to this project:
    // 1. If the secrets.properties file does not exist, create it in the same folder as the local.properties file.
    // 2. Add this line, where YOUR_API_KEY is your API key:
    //        MAPS_API_KEY=YOUR_API_KEY
    propertiesFileName = "secrets.properties"

    // A properties file containing default secret values. This file can be
    // checked in version control.
    defaultPropertiesFileName = "local.defaults.properties"
}
