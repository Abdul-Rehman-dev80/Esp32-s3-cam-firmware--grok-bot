#ifndef DIGILOG_FACE_DISPLAY_H_
#define DIGILOG_FACE_DISPLAY_H_

#include "display/oled_display.h"
#include <esp_timer.h>
#include <string>

// Robot face emotion states
enum class DigilogEmotion {
    kNeutral,
    kHappy,
    kSad,
    kThinking,
    kSurprised,
    kAngry,
    kConfused,
    kWinking,
    kSleeping,
    kListening,
    kSpeaking,
};

class DigilogFaceDisplay : public OledDisplay {
public:
    DigilogFaceDisplay(esp_lcd_panel_io_handle_t panel_io,
                       esp_lcd_panel_handle_t panel,
                       int width, int height,
                       bool mirror_x, bool mirror_y);
    virtual ~DigilogFaceDisplay();

    // Override display methods
    virtual void SetupUI() override;
    virtual void SetEmotion(const std::string& emotion) override;
    virtual void ShowNotification(const std::string& message, int duration_ms = 3000) override;

    // Face-specific methods
    void SetFaceEmotion(DigilogEmotion emotion);
    DigilogEmotion GetCurrentEmotion() const { return current_emotion_; }

    // Animation control
    void StartAnimation();
    void StopAnimation();
    bool IsAnimating() const { return is_animating_; }

private:
    DigilogEmotion current_emotion_ = DigilogEmotion::kNeutral;
    esp_timer_handle_t anim_timer_ = nullptr;
    bool is_animating_ = false;
    int anim_frame_ = 0;

    // LVGL objects for face
    lv_obj_t* face_container_ = nullptr;
    lv_obj_t* left_eye_ = nullptr;
    lv_obj_t* right_eye_ = nullptr;
    lv_obj_t* mouth_ = nullptr;
    lv_obj_t* notification_label_ = nullptr;

    // Face drawing helpers
    void DrawFace();
    void DrawEyes(DigilogEmotion emotion, int frame);
    void DrawMouth(DigilogEmotion emotion, int frame);
    void UpdateAnimation();

    // Animation timer callback
    static void AnimationTimerCallback(void* arg);

    // Map emotion string to enum
    static DigilogEmotion StringToEmotion(const std::string& emotion);
};

#endif // DIGILOG_FACE_DISPLAY_H_
