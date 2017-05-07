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

/**
 * endstops.cpp - A singleton object to manage endstops
 */

#include "../../base.h"

// TEST_ENDSTOP: test the old and the current status of an endstop
#define TEST_ENDSTOP(ENDSTOP) (TEST(current_endstop_bits & old_endstop_bits, ENDSTOP))

Endstops endstops;

// public:

bool  Endstops::enabled = true,
      Endstops::enabled_globally =
        #if ENABLED(ENDSTOPS_ONLY_FOR_HOMING)
          (false)
        #else
          (true)
        #endif
      ;
volatile char Endstops::endstop_hit_bits; // use X_MIN, Y_MIN, Z_MIN and Z_MIN_PROBE as BIT value

#if ENABLED(Z_TWO_ENDSTOPS) || ENABLED(Z_THREE_ENDSTOPS) || ENABLED(Z_FOUR_ENDSTOPS) || ENABLED(NPR2)
  uint16_t
#else
  byte
#endif
    Endstops::current_endstop_bits = 0,
    Endstops::old_endstop_bits = 0;

#if HAS(BED_PROBE)
  volatile bool Endstops::z_probe_enabled = false;
#endif

/**
 * Class and Instance Methods
 */

void Endstops::init() {

  #if HAS(X_MIN)
    #if ENABLED(ENDSTOPPULLUP_XMIN)
      SET_INPUT_PULLUP(X_MIN_PIN);
    #else
      SET_INPUT(X_MIN_PIN);
    #endif
  #endif

  #if HAS(Y_MIN)
    #if ENABLED(ENDSTOPPULLUP_YMIN)
      SET_INPUT_PULLUP(Y_MIN_PIN);
    #else
      SET_INPUT(Y_MIN_PIN);
    #endif
  #endif

  #if HAS(Z_MIN)
    #if ENABLED(ENDSTOPPULLUP_ZMIN)
      SET_INPUT_PULLUP(Z_MIN_PIN);
    #else
      SET_INPUT(Z_MIN_PIN);
    #endif
  #endif

  #if HAS(Z2_MIN)
    #if ENABLED(ENDSTOPPULLUP_ZMIN)
      SET_INPUT_PULLUP(Z2_MIN_PIN);
    #else
      SET_INPUT(Z2_MIN_PIN);
    #endif
  #endif

  #if HAS(Z3_MIN)
    #if ENABLED(ENDSTOPPULLUP_Z3MIN)
      SET_INPUT_PULLUP(Z3_MIN_PIN);
    #else
      SET_INPUT(Z3_MIN_PIN);
    #endif
  #endif

  #if HAS(Z4_MIN)
    #if ENABLED(ENDSTOPPULLUP_Z4MIN)
      SET_INPUT_PULLUP(Z4_MIN_PIN);
    #else
      SET_INPUT(Z4_MIN_PIN);
    #endif
  #endif

  #if HAS(E_MIN)
    #if ENABLED(ENDSTOPPULLUP_EMIN)
      SET_INPUT_PULLUP(E_MIN_PIN);
    #else
      SET_INPUT(E_MIN_PIN);
    #endif
  #endif

  #if HAS(X_MAX)
    #if ENABLED(ENDSTOPPULLUP_XMAX)
      SET_INPUT_PULLUP(X_MAX_PIN);
    #else
      SET_INPUT(X_MAX_PIN);
    #endif
  #endif

  #if HAS(Y_MAX)
    #if ENABLED(ENDSTOPPULLUP_YMAX)
      SET_INPUT_PULLUP(Y_MAX_PIN);
    #else
      SET_INPUT(Y_MAX_PIN);
    #endif
  #endif

  #if HAS(Z_MAX)
    #if ENABLED(ENDSTOPPULLUP_ZMAX)
      SET_INPUT_PULLUP(Z_MAX_PIN);
    #else
      SET_INPUT(Z_MAX_PIN);
    #endif
  #endif

  #if HAS(Z2_MAX)
    #if ENABLED(ENDSTOPPULLUP_ZMAX)
      SET_INPUT_PULLUP(Z2_MAX_PIN);
    #else
      SET_INPUT(Z2_MAX_PIN);
    #endif
  #endif

  #if HAS(Z3_MAX)
    #if ENABLED(ENDSTOPPULLUP_Z3MAX)
      SET_INPUT_PULLUP(Z3_MAX_PIN);
    #else
      SET_INPUT(Z3_MAX_PIN);
    #endif
  #endif

  #if HAS(Z4_MAX)
    #if ENABLED(ENDSTOPPULLUP_Z4MAX)
      SET_INPUT_PULLUP(Z4_MAX_PIN);
    #else
      SET_INPUT(Z4_MAX_PIN);
    #endif
  #endif

  #if HAS(Z_PROBE_PIN)
    #if ENABLED(ENDSTOPPULLUP_ZPROBE)
      SET_INPUT_PULLUP(Z_PROBE_PIN);
    #else
      SET_INPUT(Z_PROBE_PIN);
    #endif
  #endif

} // Endstops::init

void Endstops::report_state() {
  if (endstop_hit_bits) {
    #if ENABLED(ULTRA_LCD)
      char chrX = ' ', chrY = ' ', chrZ = ' ', chrP = ' ';
      #define _SET_STOP_CHAR(A,C) (chr## A = C)
    #else
      #define _SET_STOP_CHAR(A,C) ;
    #endif

    #define _ENDSTOP_HIT_ECHO(A,C) do{ \
      SERIAL_MV(" " STRINGIFY(A) ":", stepper.triggered_position_mm(A ##_AXIS)); \
      _SET_STOP_CHAR(A,C); }while(0)

    #define _ENDSTOP_HIT_TEST(A,C) \
      if (TEST(endstop_hit_bits, A ##_MIN) || TEST(endstop_hit_bits, A ##_MAX)) \
        _ENDSTOP_HIT_ECHO(A,C)

    SERIAL_SM(ECHO, MSG_ENDSTOPS_HIT);
    _ENDSTOP_HIT_TEST(X, 'X');
    _ENDSTOP_HIT_TEST(Y, 'Y');
    _ENDSTOP_HIT_TEST(Z, 'Z');

    #if ENABLED(Z_PROBE_ENDSTOP)
      #define P_AXIS Z_AXIS
      if (TEST(endstop_hit_bits, Z_PROBE)) _ENDSTOP_HIT_ECHO(P, 'P');
    #endif
    SERIAL_E;

    #if ENABLED(ULTRA_LCD)
      lcd_status_printf_P(0, PSTR(MSG_LCD_ENDSTOPS " %c %c %c %c"), chrX, chrY, chrZ, chrP);
    #endif

    hit_on_purpose();

    #if ENABLED(ABORT_ON_ENDSTOP_HIT_FEATURE_ENABLED) && ENABLED(SDSUPPORT)
      if (stepper.abort_on_endstop_hit) {
        card.sdprinting = false;
        card.closeFile();
        quickstop_stepper();
        thermalManager.disable_all_heaters(); // switch off all heaters.
        thermalManager.disable_all_coolers();
      }
    #endif
  }
} // Endstops::report_state

void Endstops::M119() {
  SERIAL_EM(MSG_M119_REPORT);
  #if HAS(X_MIN)
    SERIAL_EMT(MSG_X_MIN, ((READ(X_MIN_PIN)^X_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(X_MAX)
    SERIAL_EMT(MSG_X_MAX, ((READ(X_MAX_PIN)^X_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Y_MIN)
    SERIAL_EMT(MSG_Y_MIN, ((READ(Y_MIN_PIN)^Y_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Y_MAX)
    SERIAL_EMT(MSG_Y_MAX, ((READ(Y_MAX_PIN)^Y_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z_MIN)
    SERIAL_EMT(MSG_Z_MIN, ((READ(Z_MIN_PIN)^Z_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z2_MIN)
    SERIAL_EMT(MSG_Z2_MIN, ((READ(Z2_MIN_PIN)^Z2_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z3_MIN)
    SERIAL_EMT(MSG_Z3_MIN, ((READ(Z3_MIN_PIN)^Z3_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z4_MIN)
    SERIAL_EMT(MSG_Z4_MIN, ((READ(Z4_MIN_PIN)^Z4_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z_MAX)
    SERIAL_EMT(MSG_Z_MAX, ((READ(Z_MAX_PIN)^Z_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z2_MAX)
    SERIAL_EMT(MSG_Z2_MAX, ((READ(Z2_MAX_PIN)^Z2_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z3_MAX)
    SERIAL_EMT(MSG_Z3_MAX, ((READ(Z3_MAX_PIN)^Z3_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z4_MAX)
    SERIAL_EMT(MSG_Z4_MAX, ((READ(Z4_MAX_PIN)^Z4_MAX_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(Z_PROBE_PIN)
    SERIAL_EMT(MSG_Z_PROBE, ((READ(Z_PROBE_PIN)^Z_PROBE_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(E_MIN)
    SERIAL_EMT(MSG_E_MIN, ((READ(E_MIN_PIN)^E_MIN_ENDSTOP_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
  #if HAS(FIL_RUNOUT)
    SERIAL_EMT(MSG_FILAMENT_RUNOUT_SENSOR, ((READ(FIL_RUNOUT_PIN)^FIL_RUNOUT_PIN_INVERTING) ? MSG_ENDSTOP_HIT : MSG_ENDSTOP_OPEN));
  #endif
} // Endstops::M119

#if ENABLED(Z_FOUR_ENDSTOPS)
  // Pass the result of the endstop test
  void Endstops::test_four_z_endstops(EndstopEnum es1, EndstopEnum es2, EndstopEnum es3, EndstopEnum es4) {
    byte z_test = TEST_ENDSTOP(es1) | (TEST_ENDSTOP(es2) << 1 | (TEST_ENDSTOP(es3) << 2 | (TEST_ENDSTOP(es4) << 3); // bit 0 for Z, bit 1 for Z2, bit 2 for Z3, bit 3 for Z4
    if (z_test && stepper.current_block->steps[Z_AXIS] > 0) {
      SBI(endstop_hit_bits, Z_MIN);
      if (!stepper.performing_homing || (z_test == 0xf))  //if not performing home or if both endstops were trigged during homing...
        stepper.kill_current_block();
    }
  }
#elif ENABLED(Z_THREE_ENDSTOPS)
  // Pass the result of the endstop test
  void Endstops::test_four_z_endstops(EndstopEnum es1, EndstopEnum es2, EndstopEnum es3) {
    byte z_test = TEST_ENDSTOP(es1) | (TEST_ENDSTOP(es2) << 1 | (TEST_ENDSTOP(es3) << 2); // bit 0 for Z, bit 1 for Z2, bit 2 for Z3
    if (z_test && stepper.current_block->steps[Z_AXIS] > 0) {
      SBI(endstop_hit_bits, Z_MIN);
      if (!stepper.performing_homing || (z_test == 0x7))  //if not performing home or if both endstops were trigged during homing...
        stepper.kill_current_block();
    }
  }
#elif ENABLED(Z_TWO_ENDSTOPS)
  // Pass the result of the endstop test
  void Endstops::test_two_z_endstops(EndstopEnum es1, EndstopEnum es2) {
    byte z_test = TEST_ENDSTOP(es1) | (TEST_ENDSTOP(es2) << 1); // bit 0 for Z, bit 1 for Z2
    if (z_test && stepper.current_block->steps[Z_AXIS] > 0) {
      SBI(endstop_hit_bits, Z_MIN);
      if (!stepper.performing_homing || (z_test == 0x3))  //if not performing home or if both endstops were trigged during homing...
        stepper.kill_current_block();
    }
  }
#endif

#if ENABLED(PINS_DEBUGGING)
  /**
   * monitors endstops & Z probe for changes
   *
   * If a change is detected then the LED is toggled and
   * a message is sent out the serial port
   *
   * Yes, we could miss a rapid back & forth change but
   * that won't matter because this is all manual.
   *
   */
  void Endstops::endstop_monitor() {
    static uint16_t old_endstop_bits_local = 0;
    uint16_t current_endstop_bits_local = 0;
    #if HAS_X_MIN
      if (READ(X_MIN_PIN)) SBI(current_endstop_bits_local, X_MIN);
    #endif
    #if HAS_X_MAX
      if (READ(X_MAX_PIN)) SBI(current_endstop_bits_local, X_MAX);
    #endif
    #if HAS_Y_MIN
      if (READ(Y_MIN_PIN)) SBI(current_endstop_bits_local, Y_MIN);
    #endif
    #if HAS_Y_MAX
      if (READ(Y_MAX_PIN)) SBI(current_endstop_bits_local, Y_MAX);
    #endif
    #if HAS_Z_MIN
      if (READ(Z_MIN_PIN)) SBI(current_endstop_bits_local, Z_MIN);
    #endif
    #if HAS_Z_MAX
      if (READ(Z_MAX_PIN)) SBI(current_endstop_bits_local, Z_MAX);
    #endif
    #if HAS_Z_PROBE_PIN
      if (READ(Z_PROBE_PIN)) SBI(current_endstop_bits_local, Z_PROBE);
    #endif
    #if HAS_Z2_MIN
      if (READ(Z2_MIN_PIN)) SBI(current_endstop_bits_local, Z2_MIN);
    #endif
    #if HAS_Z2_MAX
      if (READ(Z2_MAX_PIN)) SBI(current_endstop_bits_local, Z2_MAX);
    #endif

    uint16_t endstop_change = current_endstop_bits_local ^ old_endstop_bits_local;

    if (endstop_change) {
      #if HAS_X_MIN
        if (TEST(endstop_change, X_MIN)) SERIAL_MV("X_MIN:", !!TEST(current_endstop_bits_local, X_MIN));
      #endif
      #if HAS_X_MAX
        if (TEST(endstop_change, X_MAX)) SERIAL_MV("  X_MAX:", !!TEST(current_endstop_bits_local, X_MAX));
      #endif
      #if HAS_Y_MIN
        if (TEST(endstop_change, Y_MIN)) SERIAL_MV("  Y_MIN:", !!TEST(current_endstop_bits_local, Y_MIN));
      #endif
      #if HAS_Y_MAX
        if (TEST(endstop_change, Y_MAX)) SERIAL_MV("  Y_MAX:", !!TEST(current_endstop_bits_local, Y_MAX));
      #endif
      #if HAS_Z_MIN
        if (TEST(endstop_change, Z_MIN)) SERIAL_MV("  Z_MIN:", !!TEST(current_endstop_bits_local, Z_MIN));
      #endif
      #if HAS_Z_MAX
        if (TEST(endstop_change, Z_MAX)) SERIAL_MV("  Z_MAX:", !!TEST(current_endstop_bits_local, Z_MAX));
      #endif
      #if HAS_Z_PROBE_PIN
        if (TEST(endstop_change, Z_PROBE)) SERIAL_MV("  PROBE:", !!TEST(current_endstop_bits_local, Z_PROBE));
      #endif
      #if HAS_Z2_MIN
        if (TEST(endstop_change, Z2_MIN)) SERIAL_MV("  Z2_MIN:", !!TEST(current_endstop_bits_local, Z2_MIN));
      #endif
      #if HAS_Z2_MAX
        if (TEST(endstop_change, Z2_MAX)) SERIAL_MV("  Z2_MAX:", !!TEST(current_endstop_bits_local, Z2_MAX));
      #endif
      SERIAL_M("\n\n");
      old_endstop_bits_local = current_endstop_bits_local;
    }
  }
#endif // PINS_DEBUGGING

// Check endstops - Called from ISR!
void Endstops::update() {

  #define _ENDSTOP(AXIS, MINMAX) AXIS ##_## MINMAX
  #define _ENDSTOP_PIN(AXIS, MINMAX) AXIS ##_## MINMAX ##_PIN
  #define _ENDSTOP_INVERTING(AXIS, MINMAX) AXIS ##_## MINMAX ##_ENDSTOP_INVERTING
  #define _ENDSTOP_HIT(AXIS) SBI(endstop_hit_bits, _ENDSTOP(AXIS, MIN))

  // UPDATE_ENDSTOP_BIT: set the current endstop bits for an endstop to its status
  #define UPDATE_ENDSTOP_BIT(AXIS, MINMAX) SET_BIT(current_endstop_bits, _ENDSTOP(AXIS, MINMAX), (READ(_ENDSTOP_PIN(AXIS, MINMAX)) != _ENDSTOP_INVERTING(AXIS, MINMAX)))
  // COPY_BIT: copy the value of SRC_BIT to DST_BIT in DST
  #define COPY_BIT(DST, SRC_BIT, DST_BIT) SET_BIT(DST, DST_BIT, TEST(DST, SRC_BIT))

  #define _UPDATE_ENDSTOP(AXIS,MINMAX,CODE) do { \
      UPDATE_ENDSTOP_BIT(AXIS, MINMAX); \
      if (TEST_ENDSTOP(_ENDSTOP(AXIS, MINMAX)) && stepper.current_block->steps[_AXIS(AXIS)] > 0) { \
        _ENDSTOP_HIT(AXIS); \
        stepper.endstop_triggered(_AXIS(AXIS)); \
        CODE; \
      } \
    } while(0)

  #if ENABLED(G38_PROBE_TARGET) && PIN_EXISTS(Z_MIN)  // If G38 command then check Z_MIN for every axis and every direction

    #define UPDATE_ENDSTOP(AXIS,MINMAX) do { \
        _UPDATE_ENDSTOP(AXIS,MINMAX,NOOP); \
        if (G38_move) _UPDATE_ENDSTOP(Z, MIN, G38_endstop_hit = true); \
      } while(0)

  #else

    #define UPDATE_ENDSTOP(AXIS,MINMAX) _UPDATE_ENDSTOP(AXIS,MINMAX,NOOP)

  #endif

  #if CORE_IS_XY || CORE_IS_XZ
    #if MECH(COREYX) || MECH(COREZX)
      #define CORE_X_CMP !=
      #define CORE_X_NOT !
    #else
      #define CORE_X_CMP ==
      #define CORE_X_NOT
    #endif
    // Head direction in -X axis for CoreXY and CoreXZ bots.
    // If steps differ, both axes are moving.
    // If DeltaA == -DeltaB, the movement is only in the 2nd axis (Y or Z, handled below)
    // If DeltaA ==  DeltaB, the movement is only in the 1st axis (X)
    if (stepper.current_block->steps[CORE_AXIS_1] != stepper.current_block->steps[CORE_AXIS_2] || stepper.motor_direction(CORE_AXIS_1) CORE_X_CMP stepper.motor_direction(CORE_AXIS_2)) {
      if (CORE_X_NOT stepper.motor_direction(X_HEAD))
  #else
    if (stepper.motor_direction(X_AXIS))   // stepping along -X axis (regular Cartesian bot)
  #endif
      { // -direction
        #if ENABLED(DUAL_X_CARRIAGE)
          // with 2 x-carriages, endstops are only checked in the homing direction for the active extruder
          if ((stepper.TOOL_E_INDEX == 0 && X_HOME_DIR < 0) || (stepper.TOOL_E_INDEX != 0 && X2_HOME_DIR < 0))
        #endif
          {
            #if HAS(X_MIN)
              UPDATE_ENDSTOP(X, MIN);
            #endif
          }
      }
      else { // +direction
        #if ENABLED(DUAL_X_CARRIAGE)
          // with 2 x-carriages, endstops are only checked in the homing direction for the active extruder
          if ((stepper.TOOL_E_INDEX == 0 && X_HOME_DIR > 0) || (stepper.TOOL_E_INDEX != 0 && X2_HOME_DIR > 0))
        #endif
          {
            #if HAS(X_MAX)
              UPDATE_ENDSTOP(X, MAX);
            #endif
          }
      }
  #if CORE_IS_XY || CORE_IS_XZ
    }
  #endif

  // Handle swapped vs. typical Core axis order
  #if MECH(COREYX) || MECH(COREZY) || MECH(COREZX)
    #define CORE_YZ_CMP ==
    #define CORE_YZ_NOT !
  #elif CORE_IS_XY || CORE_IS_YZ || CORE_IS_XZ
    #define CORE_YZ_CMP !=
    #define CORE_YZ_NOT
  #endif

  #if CORE_IS_XY || CORE_IS_YZ
    // Head direction in -Y axis for CoreXY / CoreYZ bots.
    // If steps differ, both axes are moving
    // If DeltaA ==  DeltaB, the movement is only in the 1st axis (X or Y)
    // If DeltaA == -DeltaB, the movement is only in the 2nd axis (Y or Z)
    if (stepper.current_block->steps[CORE_AXIS_1] != stepper.current_block->steps[CORE_AXIS_2] || stepper.motor_direction(CORE_AXIS_1) CORE_YZ_CMP stepper.motor_direction(CORE_AXIS_2)) {
      if (CORE_YZ_NOT stepper.motor_direction(Y_HEAD))
  #else
      if (stepper.motor_direction(Y_AXIS))   // -direction
  #endif
      { // -direction
        #if HAS(Y_MIN)
          UPDATE_ENDSTOP(Y, MIN);
        #endif
      }
      else { // +direction
        #if HAS(Y_MAX)
          UPDATE_ENDSTOP(Y, MAX);
        #endif
      }
  #if CORE_IS_XY || CORE_IS_YZ
    }
  #endif

  #if CORE_IS_XZ || CORE_IS_YZ
    // Head direction in -Z axis for CoreXZ or CoreYZ bots.
    // If steps differ, both axes are moving
    // If DeltaA ==  DeltaB, the movement is only in the 1st axis (X or Y, already handled above)
    // If DeltaA == -DeltaB, the movement is only in the 2nd axis (Z)
    if (stepper.current_block->steps[CORE_AXIS_1] != stepper.current_block->steps[CORE_AXIS_2] || stepper.motor_direction(CORE_AXIS_1) CORE_YZ_CMP stepper.motor_direction(CORE_AXIS_2)) {
      if (CORE_YZ_NOT stepper.motor_direction(Z_HEAD))
  #else
      if (stepper.motor_direction(Z_AXIS))
  #endif
      { // Z -direction. Gantry down, bed up.
        #if HAS(Z_MIN)

          #if ENABLED(Z_FOUR_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MIN);
            #if HAS(Z2_MIN)
              UPDATE_ENDSTOP_BIT(Z2, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z2_MIN);
            #endif
            #if HAS(Z3_MIN)
              UPDATE_ENDSTOP_BIT(Z3, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z3_MIN);
            #endif
            #if HAS(Z4_MIN)
              UPDATE_ENDSTOP_BIT(Z4, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z4_MIN);
            #endif

            test_four_z_endstops(Z_MIN, Z2_MIN, Z3_MIN, Z4_MIN);

          #elif ENABLED(Z_THREE_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MIN);
            #if HAS(Z2_MIN)
              UPDATE_ENDSTOP_BIT(Z2, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z2_MIN);
            #endif
            #if HAS(Z3_MIN)
              UPDATE_ENDSTOP_BIT(Z3, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z3_MIN);
            #endif

            test_three_z_endstops(Z_MIN, Z2_MIN, Z3_MIN);

          #elif ENABLED(Z_TWO_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MIN);
            #if HAS(Z2_MIN)
              UPDATE_ENDSTOP_BIT(Z2, MIN);
            #else
              COPY_BIT(current_endstop_bits, Z_MIN, Z2_MIN);
            #endif

            test_two_z_endstops(Z_MIN, Z2_MIN);

          #else

            #if HAS(BED_PROBE) && HASNT(Z_PROBE_PIN)
              if (z_probe_enabled) UPDATE_ENDSTOP(Z, MIN);
            #else
              UPDATE_ENDSTOP(Z, MIN);
            #endif

          #endif
        #endif // HAS_Z_MIN

        // When closing the gap check the enabled probe
        #if HAS(BED_PROBE) && HAS(Z_PROBE_PIN)
          if (z_probe_enabled) {
            UPDATE_ENDSTOP(Z, PROBE);
            if (TEST_ENDSTOP(Z_PROBE)) SBI(endstop_hit_bits, Z_PROBE);
          }
        #endif
      }
      else { // Z +direction. Gantry up, bed down.
        #if HAS(Z_MAX)

          #if ENABLED(Z_FOUR_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MAX);
            #if HAS(Z2_MAX)
              UPDATE_ENDSTOP_BIT(Z2, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z2_MAX);
            #endif
            #if HAS(Z3_MAX)
              UPDATE_ENDSTOP_BIT(Z3, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z3_MAX);
            #endif
            #if HAS(Z4_MAX)
              UPDATE_ENDSTOP_BIT(Z4, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z4_MAX);
            #endif

            test_four_z_endstops(Z_MAX, Z2_MAX, Z3_MAX, Z4_MAX);

          #elif ENABLED(Z_THREE_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MAX);
            #if HAS(Z2_MAX)
              UPDATE_ENDSTOP_BIT(Z2, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z2_MAX);
            #endif
            #if HAS(Z3_MAX)
              UPDATE_ENDSTOP_BIT(Z3, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z3_MAX);
            #endif

            test_three_z_endstops(Z_MAX, Z2_MAX, Z3_MAX);

          #elif ENABLED(Z_TWO_ENDSTOPS)

            UPDATE_ENDSTOP_BIT(Z, MAX);
            #if HAS(Z2_MAX)
              UPDATE_ENDSTOP_BIT(Z2, MAX);
            #else
              COPY_BIT(current_endstop_bits, Z_MAX, Z2_MAX);
            #endif

            test_two_z_endstops(Z_MAX, Z2_MAX);

          #else // !Z_TWO_ENDSTOPS

            UPDATE_ENDSTOP(Z, MAX);

          #endif // !Z_TWO_ENDSTOPS
        #endif // Z_MAX_PIN
      }
  #if CORE_IS_XZ || CORE_IS_YZ
    }
  #endif

  #if ENABLED(NPR2)
    UPDATE_ENDSTOP(E, MIN);
  #endif

  old_endstop_bits = current_endstop_bits;

} // Endstops::update()
