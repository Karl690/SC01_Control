#include "M_Codes.h"
#include "sequencer.h"
void M_Code_M104(void) {
    ESP_LOGI(TAG, "M_Code_M104");
    if (ARG_S_MISSING) { 
        ESP_LOGI(TAG, "M_Code_M104: Missing S Argment");
        return;
    }
    systemconfig.pcnt.enabled = ARG_S;
}