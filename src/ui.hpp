#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include "Arduino.h"

static lv_obj_t *ui_speedArc;
static lv_obj_t *ui_speedLabel;
static lv_obj_t *ui_warningsLabel;
static lv_obj_t *ui_fuelTrimChart;
static lv_chart_series_t *ui_fuelTrimChartBank1Series;
static lv_chart_series_t *ui_fuelTrimChartBank2Series;
static lv_obj_t *ui_stft1Label;
static lv_obj_t *ui_stft2Label;
static lv_obj_t *ui_ltft1Label;
static lv_obj_t *ui_ltft2Label;

void ui_setSpeedValue(int32_t value)
{
    lv_arc_set_value(ui_speedArc, value);
    lv_label_set_text(ui_speedLabel, String(value).c_str());
}

void ui_updateWarningLabel(const char* text, bool prepend = false) {
    if (prepend) {
        lv_label_ins_text(ui_warningsLabel, 0, text);
    } else {
        lv_label_set_text(ui_warningsLabel, text);
    }
}

void ui_updateFuelTrimChart(int bank1Value, int bank2Value)
{
    lv_chart_set_next_value(ui_fuelTrimChart, ui_fuelTrimChartBank1Series, bank1Value);
    lv_chart_set_next_value(ui_fuelTrimChart, ui_fuelTrimChartBank2Series, bank2Value);
}

inline void updateFuelTrimLabel(lv_obj_t* label, const char* trimName, float trim)
{
    if (abs(trim) > 10.0f) {
        lv_obj_set_style_text_color(label, lv_color_hex(0xFAC500), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
        lv_obj_set_style_text_color(ui_ltft1Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    int trim_int = trim;
    lv_label_set_text(label, (String(trimName) + " " + trim_int).c_str());
}

void ui_updateStft1Label(float value)
{
    updateFuelTrimLabel(ui_stft1Label,"STFT1", value);
}

void ui_updateStft2Label(float value)
{
    updateFuelTrimLabel(ui_stft2Label,"STFT2", value);
}

void ui_updateLtft1Label(float value)
{
    updateFuelTrimLabel(ui_ltft1Label,"LTFT1", value);
}

void ui_updateLtft2Label(float value)
{
    updateFuelTrimLabel(ui_ltft2Label,"LTFT2", value);
}


LilyGo_Class amoled;

void ui_setup()
{
    bool rslt =  amoled.beginAMOLED_191();

    if (!rslt) {
        while (1) {
            Serial.println("The board model cannot be detected, please raise the Core Debug Level to an error");
            delay(1000);
        }
    }

    beginLvglHelper(amoled);

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), LV_STATE_DEFAULT);

    ui_warningsLabel = lv_label_create(lv_scr_act());
    lv_obj_set_width(ui_warningsLabel, 218);
    lv_obj_set_height(ui_warningsLabel, LV_SIZE_CONTENT);    /// 56
    lv_obj_set_x(ui_warningsLabel, 14);
    lv_obj_set_y(ui_warningsLabel, -6);
    lv_obj_set_align(ui_warningsLabel, LV_ALIGN_BOTTOM_LEFT);
    lv_label_set_text(ui_warningsLabel, "DTCS: P0123 P0234 P123 P500 P600 P1203");
    lv_obj_set_style_text_color(ui_warningsLabel, lv_color_hex(0xFAC500), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_warningsLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_warningsLabel, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_fuelTrimChart = lv_chart_create(lv_scr_act());
    lv_obj_set_width(ui_fuelTrimChart, 257);
    lv_obj_set_height(ui_fuelTrimChart, 153);
    lv_obj_set_x(ui_fuelTrimChart, 103);
    lv_obj_set_y(ui_fuelTrimChart, -32);
    lv_obj_set_align(ui_fuelTrimChart, LV_ALIGN_CENTER);
    lv_chart_set_type(ui_fuelTrimChart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(ui_fuelTrimChart, 50);
    lv_chart_set_range(ui_fuelTrimChart, LV_CHART_AXIS_PRIMARY_Y, -20, 20);
    lv_chart_set_range(ui_fuelTrimChart, LV_CHART_AXIS_SECONDARY_Y, -20, 20);
    lv_chart_set_div_line_count(ui_fuelTrimChart, 0, 0);
    lv_chart_set_axis_tick(ui_fuelTrimChart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 0, 0, false, 50);
    lv_chart_set_axis_tick(ui_fuelTrimChart, LV_CHART_AXIS_PRIMARY_Y, 10, 5, 5, 2, false, 50);
    lv_chart_set_axis_tick(ui_fuelTrimChart, LV_CHART_AXIS_SECONDARY_Y, 10, 5, 5, 2, true, 25);
    lv_chart_series_t * ui_fuelTrimChart_series_1 = lv_chart_add_series(ui_fuelTrimChart, lv_color_hex(0x44FF58),
                                                                        LV_CHART_AXIS_SECONDARY_Y);
    static lv_coord_t ui_fuelTrimChart_series_1_array[] = { 15, 20, -5, 5, 0 };
    lv_chart_set_ext_y_array(ui_fuelTrimChart, ui_fuelTrimChart_series_1, ui_fuelTrimChart_series_1_array);
    lv_chart_series_t * ui_fuelTrimChart_series_2 = lv_chart_add_series(ui_fuelTrimChart, lv_color_hex(0xFF1313),
                                                                        LV_CHART_AXIS_SECONDARY_Y);
    static lv_coord_t ui_fuelTrimChart_series_2_array[] = { 0, -2, 2, 1, 0, 3, -5, 2 };
    lv_chart_set_ext_y_array(ui_fuelTrimChart, ui_fuelTrimChart_series_2, ui_fuelTrimChart_series_2_array);
    lv_obj_set_style_radius(ui_fuelTrimChart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_fuelTrimChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_fuelTrimChart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_fuelTrimChart, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui_fuelTrimChart, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_fuelTrimChart, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui_fuelTrimChart, LV_BORDER_SIDE_FULL, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_line_color(ui_fuelTrimChart, lv_color_hex(0xFFFFFF), LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui_fuelTrimChart, 255, LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui_fuelTrimChart, 0, LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_fuelTrimChart, lv_color_hex(0xFFFFFF), LV_PART_TICKS | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_fuelTrimChart, 255, LV_PART_TICKS | LV_STATE_DEFAULT);

    lv_obj_set_style_size(ui_fuelTrimChart, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    ui_speedArc = lv_arc_create(lv_scr_act());
    lv_obj_set_width(ui_speedArc, 220);
    lv_obj_set_height(ui_speedArc, 233);
    lv_obj_set_x(ui_speedArc, 10);
    lv_obj_set_y(ui_speedArc, 7);
    lv_arc_set_range(ui_speedArc, 0, 180);
    lv_arc_set_value(ui_speedArc, 35);
    lv_arc_set_bg_angles(ui_speedArc, 150, 30);
    lv_obj_set_style_arc_color(ui_speedArc, lv_color_hex(0x8D8D8D), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_speedArc, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_speedArc, 25, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_speedArc, false, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_arc_color(ui_speedArc, lv_color_hex(0x3BB8FF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_opa(ui_speedArc, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_width(ui_speedArc, 25, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_arc_rounded(ui_speedArc, false, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_set_style_bg_color(ui_speedArc, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_speedArc, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

    ui_speedLabel = lv_label_create(ui_speedArc);
    lv_obj_set_width(ui_speedLabel, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_speedLabel, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_speedLabel, 0);
    lv_obj_set_y(ui_speedLabel, -33);
    lv_obj_set_align(ui_speedLabel, LV_ALIGN_CENTER);
    lv_label_set_text(ui_speedLabel, "100");
    lv_obj_set_style_text_color(ui_speedLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_speedLabel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_speedLabel, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_speedLabel, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* ui_Label8 = lv_label_create(ui_speedArc);
    lv_obj_set_width(ui_Label8, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Label8, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Label8, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label8, "km/h");
    lv_obj_set_style_text_color(ui_Label8, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label8, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ltft1Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(ui_ltft1Label, 140);
    lv_obj_set_height(ui_ltft1Label, LV_SIZE_CONTENT);    /// 64
    lv_obj_set_x(ui_ltft1Label, 43);
    lv_obj_set_y(ui_ltft1Label, 68);
    lv_obj_set_align(ui_ltft1Label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_ltft1Label, "LTFT1 15%");
    lv_obj_set_style_text_color(ui_ltft1Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ltft1Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_ltft1Label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ltft1Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_stft1Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(ui_stft1Label, 140);
    lv_obj_set_height(ui_stft1Label, LV_SIZE_CONTENT);    /// 64
    lv_obj_set_x(ui_stft1Label, 194);
    lv_obj_set_y(ui_stft1Label, 69);
    lv_obj_set_align(ui_stft1Label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_stft1Label, "STFT1 5%");
    lv_obj_set_style_text_color(ui_stft1Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_stft1Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_stft1Label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_stft1Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_stft2Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(ui_stft2Label, 140);
    lv_obj_set_height(ui_stft2Label, LV_SIZE_CONTENT);    /// 64
    lv_obj_set_x(ui_stft2Label, 194);
    lv_obj_set_y(ui_stft2Label, 104);
    lv_obj_set_align(ui_stft2Label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_stft2Label, "STFT2 -25%");
    lv_obj_set_style_text_color(ui_stft2Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_stft2Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_stft2Label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_stft2Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_ltft2Label = lv_label_create(lv_scr_act());
    lv_obj_set_width(ui_ltft2Label, 140);
    lv_obj_set_height(ui_ltft2Label, LV_SIZE_CONTENT);    /// 64
    lv_obj_set_x(ui_ltft2Label, 43);
    lv_obj_set_y(ui_ltft2Label, 104);
    lv_obj_set_align(ui_ltft2Label, LV_ALIGN_CENTER);
    lv_label_set_text(ui_ltft2Label, "LTFT2 -5%");
    lv_obj_set_style_text_color(ui_ltft2Label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_ltft2Label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_ltft2Label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_ltft2Label, &lv_font_montserrat_24, LV_PART_MAIN | LV_STATE_DEFAULT);

    // My customization
    ui_fuelTrimChartBank1Series = ui_fuelTrimChart_series_1;
    ui_fuelTrimChartBank2Series = ui_fuelTrimChart_series_2;
    lv_chart_set_update_mode(ui_fuelTrimChart, LV_CHART_UPDATE_MODE_SHIFT);

    static lv_coord_t series1_array[50] = {0};
    static lv_coord_t series2_array[50] = {0};
    lv_chart_set_ext_y_array(ui_fuelTrimChart, ui_fuelTrimChart_series_1, series1_array);
    lv_chart_set_ext_y_array(ui_fuelTrimChart, ui_fuelTrimChart_series_2, series2_array);
}

void ui_loop() {
    lv_task_handler();
}