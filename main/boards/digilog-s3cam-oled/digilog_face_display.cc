#include "digilog_face_display.h"
#include "config.h"
#include <esp_log.h>
#include <cstring>
#include <algorithm>

static const char* TAG = "DigilogFaceDisplay";

DigilogFaceDisplay::DigilogFaceDisplay(esp_lcd_panel_io_handle_t panel_io,
                                       esp_lcd_panel_handle_t panel,
                                       int width, int height,
                                       bool mirror_x, bool mirror_y)
    : OledDisplay(panel_io, panel, width, height, mirror_x, mirror_y) {
    ESP_LOGI(TAG, "Creating DigilogFace display %dx%d", width, height);
}

DigilogFaceDisplay::~DigilogFaceDisplay() {
    StopAnimation();
}

void DigilogFaceDisplay::SetupUI() {
    ESP_LOGI(TAG, "Setting up DigilogFace UI");

    // Create face container
    face_container_ = lv_obj_create(lv_scr_act());
    lv_obj_set_size(face_container_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(face_container_, lv_color_black(), 0);
    lv_obj_set_style_border_width(face_container_, 0, 0);
    lv_obj_set_style_pad_all(face_container_, 0, 0);
    lv_obj_center(face_container_);

    // Create left eye
    left_eye_ = lv_obj_create(face_container_);
    lv_obj_set_size(left_eye_, 24, 24);
    lv_obj_set_style_radius(left_eye_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(left_eye_, lv_color_white(), 0);
    lv_obj_set_style_border_width(left_eye_, 0, 0);
    lv_obj_set_pos(left_eye_, 28, 16);

    // Create right eye
    right_eye_ = lv_obj_create(face_container_);
    lv_obj_set_size(right_eye_, 24, 24);
    lv_obj_set_style_radius(right_eye_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(right_eye_, lv_color_white(), 0);
    lv_obj_set_style_border_width(right_eye_, 0, 0);
    lv_obj_set_pos(right_eye_, 76, 16);

    // Create mouth (arc or line depending on emotion)
    mouth_ = lv_obj_create(face_container_);
    lv_obj_set_size(mouth_, 40, 12);
    lv_obj_set_style_radius(mouth_, 6, 0);
    lv_obj_set_style_bg_color(mouth_, lv_color_white(), 0);
    lv_obj_set_style_border_width(mouth_, 0, 0);
    lv_obj_set_pos(mouth_, 44, 48);

    // Create notification label (hidden by default)
    notification_label_ = lv_label_create(face_container_);
    lv_obj_set_style_text_color(notification_label_, lv_color_white(), 0);
    lv_label_set_text(notification_label_, "");
    lv_obj_add_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(notification_label_, LV_ALIGN_BOTTOM_MID, 0, -4);

    // Start animation
    StartAnimation();

    ESP_LOGI(TAG, "DigilogFace UI setup complete");
}

void DigilogFaceDisplay::SetEmotion(const std::string& emotion) {
    DigilogEmotion new_emotion = StringToEmotion(emotion);
    SetFaceEmotion(new_emotion);
}

void DigilogFaceDisplay::SetFaceEmotion(DigilogEmotion emotion) {
    if (emotion == current_emotion_) return;

    ESP_LOGD(TAG, "Setting emotion: %d", static_cast<int>(emotion));
    current_emotion_ = emotion;
    anim_frame_ = 0;
    DrawFace();
}

void DigilogFaceDisplay::ShowNotification(const std::string& message, int duration_ms) {
    if (!notification_label_) return;

    lv_label_set_text(notification_label_, message.c_str());
    lv_obj_clear_flag(notification_label_, LV_OBJ_FLAG_HIDDEN);

    // Auto-hide after duration (simplified - in production use lv_timer)
    // For now just show it
}

void DigilogFaceDisplay::StartAnimation() {
    if (is_animating_) return;

    esp_timer_create_args_t timer_args = {
        .callback = AnimationTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "digilog_face_anim",
        .skip_unhandled_events = true,
    };

    if (esp_timer_create(&timer_args, &anim_timer_) == ESP_OK) {
        esp_timer_start_periodic(anim_timer_, DIGILOG_FACE_ANIM_PERIOD_MS * 1000);
        is_animating_ = true;
        ESP_LOGI(TAG, "Face animation started");
    }
}

void DigilogFaceDisplay::StopAnimation() {
    if (!is_animating_) return;

    if (anim_timer_) {
        esp_timer_stop(anim_timer_);
        esp_timer_delete(anim_timer_);
        anim_timer_ = nullptr;
    }
    is_animating_ = false;
    ESP_LOGI(TAG, "Face animation stopped");
}

void DigilogFaceDisplay::AnimationTimerCallback(void* arg) {
    auto* self = static_cast<DigilogFaceDisplay*>(arg);
    self->UpdateAnimation();
}

void DigilogFaceDisplay::UpdateAnimation() {
    anim_frame_++;
    if (anim_frame_ > 100) anim_frame_ = 0;

    DrawFace();
}

void DigilogFaceDisplay::DrawFace() {
    DrawEyes(current_emotion_, anim_frame_);
    DrawMouth(current_emotion_, anim_frame_);
}

void DigilogFaceDisplay::DrawEyes(DigilogEmotion emotion, int frame) {
    if (!left_eye_ || !right_eye_) return;

    int eye_height = 24;
    int eye_y = 16;
    lv_coord_t radius = LV_RADIUS_CIRCLE;

    switch (emotion) {
        case DigilogEmotion::kHappy:
            // Squinted happy eyes (arcs)
            eye_height = 8;
            eye_y = 24;
            radius = 4;
            break;

        case DigilogEmotion::kSad:
            // Droopy eyes
            eye_height = 16;
            eye_y = 20;
            break;

        case DigilogEmotion::kSurprised:
            // Wide open eyes
            eye_height = 28;
            eye_y = 14;
            break;

        case DigilogEmotion::kAngry:
            // Narrowed angry eyes
            eye_height = 12;
            eye_y = 22;
            radius = 2;
            break;

        case DigilogEmotion::kSleeping:
            // Closed eyes (thin lines)
            eye_height = 4;
            eye_y = 28;
            radius = 2;
            break;

        case DigilogEmotion::kThinking:
            // One eye bigger (asymmetric)
            lv_obj_set_size(left_eye_, 20, 20);
            lv_obj_set_size(right_eye_, 24, 24);
            return;

        case DigilogEmotion::kWinking:
            // Left eye normal, right eye closed
            lv_obj_set_size(left_eye_, 24, 24);
            lv_obj_set_size(right_eye_, 24, 4);
            lv_obj_set_pos(right_eye_, 76, 26);
            return;

        case DigilogEmotion::kListening:
            // Slightly larger, attentive
            eye_height = 26;
            break;

        case DigilogEmotion::kSpeaking:
            // Normal with slight pulsing based on frame
            eye_height = 24 + (frame % 4) - 2;
            break;

        default:
            // Neutral - blink occasionally
            if (frame % 50 == 0) {
                eye_height = 4;  // Blink
            }
            break;
    }

    lv_obj_set_size(left_eye_, 24, eye_height);
    lv_obj_set_size(right_eye_, 24, eye_height);
    lv_obj_set_pos(left_eye_, 28, eye_y);
    lv_obj_set_pos(right_eye_, 76, eye_y);
    lv_obj_set_style_radius(left_eye_, radius, 0);
    lv_obj_set_style_radius(right_eye_, radius, 0);
}

void DigilogFaceDisplay::DrawMouth(DigilogEmotion emotion, int frame) {
    if (!mouth_) return;

    int mouth_width = 40;
    int mouth_height = 12;
    int mouth_x = 44;
    int mouth_y = 48;
    lv_coord_t radius = 6;

    switch (emotion) {
        case DigilogEmotion::kHappy:
            // Wide smile
            mouth_width = 50;
            mouth_height = 16;
            mouth_x = 39;
            mouth_y = 44;
            radius = 8;
            break;

        case DigilogEmotion::kSad:
            // Downturned (small inverted arc)
            mouth_width = 30;
            mouth_height = 8;
            mouth_x = 49;
            mouth_y = 52;
            radius = 4;
            break;

        case DigilogEmotion::kSurprised:
            // Open O shape
            mouth_width = 20;
            mouth_height = 20;
            mouth_x = 54;
            mouth_y = 42;
            radius = LV_RADIUS_CIRCLE;
            break;

        case DigilogEmotion::kAngry:
            // Tight line
            mouth_width = 30;
            mouth_height = 4;
            mouth_y = 52;
            radius = 2;
            break;

        case DigilogEmotion::kSleeping:
            // Small neutral
            mouth_width = 20;
            mouth_height = 6;
            mouth_x = 54;
            break;

        case DigilogEmotion::kThinking:
            // Off-center small
            mouth_width = 15;
            mouth_height = 8;
            mouth_x = 60;
            break;

        case DigilogEmotion::kSpeaking:
            // Animated open/close
            mouth_height = 8 + (frame % 8);
            mouth_y = 46 - (frame % 4);
            break;

        default:
            // Neutral
            break;
    }

    lv_obj_set_size(mouth_, mouth_width, mouth_height);
    lv_obj_set_pos(mouth_, mouth_x, mouth_y);
    lv_obj_set_style_radius(mouth_, radius, 0);
}

DigilogEmotion DigilogFaceDisplay::StringToEmotion(const std::string& emotion) {
    std::string lower = emotion;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower == "happy" || lower == "laughing" || lower == "funny" ||
        lower == "loving" || lower == "confident" || lower == "delicious") {
        return DigilogEmotion::kHappy;
    }
    if (lower == "sad" || lower == "crying") {
        return DigilogEmotion::kSad;
    }
    if (lower == "thinking" || lower == "confused") {
        return DigilogEmotion::kThinking;
    }
    if (lower == "surprise" || lower == "surprised" || lower == "shocked") {
        return DigilogEmotion::kSurprised;
    }
    if (lower == "angry" || lower == "embarrassed") {
        return DigilogEmotion::kAngry;
    }
    if (lower == "winking" || lower == "relaxed") {
        return DigilogEmotion::kWinking;
    }
    if (lower == "sleeping") {
        return DigilogEmotion::kSleeping;
    }
    if (lower == "listening") {
        return DigilogEmotion::kListening;
    }
    if (lower == "speaking") {
        return DigilogEmotion::kSpeaking;
    }

    return DigilogEmotion::kNeutral;
}
