/**
 * MK4duo 3D Printer Firmware
 *
 * Based on Marlin, Sprinter and grbl
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 * Copyright (C) 2013 - 2016 Alberto Cotronei @MagoKimbra
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EEPROM_H
#define EEPROM_H

class EEPROM {

  public:

    static void Factory_Settings();
    static bool Store_Settings();

    #if DISABLED(DISABLE_M503)
      static void Print_Settings(bool forReplay = false);
    #else
      static inline void Print_Settings(bool forReplay = false) {}
    #endif

    #if ENABLED(EEPROM_SETTINGS)
      static bool Load_Settings();
    #else
      static inline bool Load_Settings() { Factory_Settings(); Print_Settings(); return true; }
    #endif

  private:

    static void writeData(int &pos, const uint8_t* value, uint16_t size);
    static void readData(int &pos, uint8_t* value, uint16_t size);
    static void Postprocess();

    static uint16_t eeprom_checksum;
    static const char version[6];

};

extern EEPROM eeprom;

#endif //CONFIGURATION_STORE_H
