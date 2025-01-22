#ifndef _Boardinfo_H_
#define _Boardinfo_H_

/**
 * @file BoardInfo.h
 * @author Gerry Sebb
 * @brief Hardwareinfo from ESP Board
 * @version 1.0
 * @date 2024-01-22
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>

class BoardInfo
{
public:
    BoardInfo();

    void ShowChipID();
    void ShowChipInfo();
    void ShowChipTemperature();

    String ShowChipIDtoString();
    
protected:
    uint64_t m_chipid; 
    esp_chip_info_t m_chipinfo; 
};

#endif