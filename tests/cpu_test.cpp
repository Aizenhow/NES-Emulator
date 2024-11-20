// cpu_test.cpp
#include "bus.h"
#include "cpu.h"
#include "json.hpp"
#include <cstdint>
#include <fstream>
#include <gtest/gtest.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

// forward declarations
auto extractTestsFromJson( const std::string &path ) -> json;
void printTestStartMsg( const std::string &testName );
void printTestEndMsg( const std::string &testName );

class CPUTestFixture : public ::testing::Test
// This class is a test fixture that provides shared setup and teardown for all tests
{
  protected:
    CPU cpu; // NOLINT
    Bus bus; // NOLINT

    // All tests assume flat memory model, which is why true is passed to Bus constructor
    CPUTestFixture() : cpu( &bus ), bus( true ) {}

    void        RunTestCase( const json &testCase );
    void        LoadStateFromJson( const json &jsonData, const std::string &state );
    std::string GetCPUStateString( const json &jsonData, const std::string &state );

    void SetFlags( u8 flag )
    {
        cpu.SetFlags( flag ); // Private method
    }

    void ClearFlags( u8 flag )
    {
        cpu.ClearFlags( flag ); // Private method
    }

    bool IsFlagSet( u8 flag )
    {
        return cpu.IsFlagSet( flag ); // Private method
    }
    u8 Read( u16 address )
    {
        return cpu.Read( address ); // Private method
    }
    void Write( u16 address, u8 data )
    {
        cpu.Write( address, data ); // Private method
    }
};

// -----------------------------------------------------------------------------
// --------------------------- GENERAL TESTS CASES -----------------------------
//           Put anything here that doesn't neatly fit into a category
// -----------------------------------------------------------------------------
TEST_F( CPUTestFixture, SanityCheck )
{
    // cpu.read and cpu.write shouldn't throw any errors
    u8 const test_val = Read( 0x0000 );
    Write( 0x0000, test_val );
}

TEST_F( CPUTestFixture, StatusFlags )
{
    u8 const carry = 0b00000001;
    u8 const zero = 0b00000010;
    u8 const interrupt_disable = 0b00000100;
    u8 const decimal = 0b00001000;
    u8 const break_flag = 0b00010000;
    u8 const unused = 0b00100000;
    u8 const overflow = 0b01000000;
    u8 const negative = 0b10000000;

    // Set and clear methods
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | unused );
    SetFlags( carry );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | unused );
    SetFlags( zero );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | unused );
    SetFlags( interrupt_disable );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | interrupt_disable | unused );
    SetFlags( decimal );
    EXPECT_EQ( cpu.GetStatusRegister(),
               0x00 | carry | zero | interrupt_disable | decimal | unused );
    SetFlags( break_flag );
    EXPECT_EQ( cpu.GetStatusRegister(),
               0x00 | carry | zero | interrupt_disable | decimal | break_flag | unused );
    ClearFlags( carry | zero | interrupt_disable | decimal | break_flag | unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 );
    SetFlags( overflow );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | overflow );
    SetFlags( negative );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | overflow | negative );
    // set all flags
    SetFlags( carry | zero | interrupt_disable | decimal | break_flag | overflow | negative |
              unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 | carry | zero | interrupt_disable | decimal |
                                            break_flag | overflow | negative | unused );
    // clear all flags
    ClearFlags( carry | zero | interrupt_disable | decimal | break_flag | overflow | negative |
                unused );
    EXPECT_EQ( cpu.GetStatusRegister(), 0x00 );

    // IsFlagSet method
    EXPECT_FALSE( IsFlagSet( carry ) );
    SetFlags( carry );
    EXPECT_TRUE( IsFlagSet( carry ) );
    EXPECT_FALSE( IsFlagSet( zero ) );
    SetFlags( zero );
    EXPECT_TRUE( IsFlagSet( zero ) );
    EXPECT_TRUE( IsFlagSet( carry | zero ) );
}

// -----------------------------------------------------------------------------
// --------------------------- ADDRESSING MODE TESTS ---------------------------
// -----------------------------------------------------------------------------
// TODO: Add addressing mode tests here

/* -----------------------------------------------------------------------------
   --------------------------- OPCODE JSON TESTS -------------------------------
                            Tom Harte's json tests.
   -----------------------------------------------------------------------------
*/

/* This is a macro to simplify test creation for json tests
 * The naming convention is <opcode hex>_<mnemonic>_<addressing mode>
 * e.g. x00_BRK_Implied, x01_ORA_IndirectX, x05_ORA_ZeroPage, etc.
 */
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define CPU_TEST( opcode_hex, mnemonic, addr_mode, filename )                                      \
    TEST_F( CPUTestFixture, x##opcode_hex##_##mnemonic##_##addr_mode )                             \
    {                                                                                              \
        std::string const testName = #opcode_hex " " #mnemonic " " #addr_mode;                     \
        printTestStartMsg( testName );                                                             \
        json const testCases = extractTestsFromJson( "tests/json/" filename );                     \
        for ( const auto &testCase : testCases )                                                   \
        {                                                                                          \
            RunTestCase( testCase );                                                               \
        }                                                                                          \
        printTestEndMsg( testName );                                                               \
    }
// NOLINTEND(cppcoreguidelines-macro-usage)

/*
  Testing an opcode:
  1. The opcode is implemented in the CPU
  2. The JSON file exists in tests/json
  3. Uncomment the corresponding test below
  4. Build and run the tests

To isolate a test from the CLI:
  ./scripts/test.sh "CPUTestFixture.x00" # runs only the BRK test
  ./scripts/test.sh "CPUTestFixture.x01" # runs only the ORA IndirectX test
  ./scripts/test.sh "CPUTestFixture.x05" # runs only the ORA ZeroPage test

  or:
  ctest -R "CPUTestFixture.x00" # from the build directory

To run all tests:
  ./scripts/test.sh
  or
  ctest # from the build directory
*/

/* CPU_TEST( SAMPLE, JSON, SANITY_CHECK, "temp.json" ); */
/* CPU_TEST( 00, BRK, Implied, "00.json" ); */
/* CPU_TEST( 01, ORA, IndirectX, "01.json" ); */
/* CPU_TEST( 05, ORA, ZeroPage, "05.json" ); */
/* CPU_TEST( 06, ASL, ZeroPage, "06.json" ); */
/* CPU_TEST( 08, PHP, Implied, "08.json" ); */
/* CPU_TEST( 09, ORA, Immediate, "09.json" ); */
/* CPU_TEST( 0A, ASL, Accumulator, "0a.json" ); */
/* CPU_TEST( 0D, ORA, Absolute, "0d.json" ); */
/* CPU_TEST( 0E, ASL, Absolute, "0e.json" ); */
/* CPU_TEST( 10, BPL, Relative, "10.json" ); */
/* CPU_TEST( 11, ORA, IndirectY, "11.json" ); */
/* CPU_TEST( 15, ORA, ZeroPageX, "15.json" ); */
/* CPU_TEST( 16, ASL, ZeroPageX, "16.json" ); */
/* CPU_TEST( 18, CLC, Implied, "18.json" ); */
/* CPU_TEST( 19, ORA, AbsoluteY, "19.json" ); */
/* CPU_TEST( 1D, ORA, AbsoluteX, "1d.json" ); */
/* CPU_TEST( 1E, ASL, AbsoluteX, "1e.json" ); */
/* CPU_TEST( 20, JSR, Absolute, "20.json" ); */
/* CPU_TEST( 21, AND, IndirectX, "21.json" ); */
/* CPU_TEST( 24, BIT, ZeroPage, "24.json" ); */
/* CPU_TEST( 25, AND, ZeroPage, "25.json" ); */
/* CPU_TEST( 26, ROL, ZeroPage, "26.json" ); */
/* CPU_TEST( 28, PLP, Implied, "28.json" ); */
/* CPU_TEST( 29, AND, Immediate, "29.json" ); */
/* CPU_TEST( 2A, ROL, Accumulator, "2a.json" ); */
/* CPU_TEST( 2C, BIT, Absolute, "2c.json" ); */
/* CPU_TEST( 2D, AND, Absolute, "2d.json" ); */
/* CPU_TEST( 2E, ROL, Absolute, "2e.json" ); */
/* CPU_TEST( 30, BMI, Relative, "30.json" ); */
/* CPU_TEST( 31, AND, IndirectY, "31.json" ); */
/* CPU_TEST( 35, AND, ZeroPageX, "35.json" ); */
/* CPU_TEST( 36, ROL, ZeroPageX, "36.json" ); */
/* CPU_TEST( 38, SEC, Implied, "38.json" ); */
/* CPU_TEST( 39, AND, AbsoluteY, "39.json" ); */
/* CPU_TEST( 3D, AND, AbsoluteX, "3d.json" ); */
/* CPU_TEST( 3E, ROL, AbsoluteX, "3e.json" ); */
/* CPU_TEST( 40, RTI, Implied, "40.json" ); */
/* CPU_TEST( 41, EOR, IndirectX, "41.json" ); */
/* CPU_TEST( 45, EOR, ZeroPage, "45.json" ); */
/* CPU_TEST( 46, LSR, ZeroPage, "46.json" ); */
/* CPU_TEST( 48, PHA, Implied, "48.json" ); */
/* CPU_TEST( 49, EOR, Immediate, "49.json" ); */
/* CPU_TEST( 4A, LSR, Accumulator, "4a.json" ); */
/* CPU_TEST( 4C, JMP, Absolute, "4c.json" ); */
/* CPU_TEST( 4D, EOR, Absolute, "4d.json" ); */
/* CPU_TEST( 4E, LSR, Absolute, "4e.json" ); */
/* CPU_TEST( 50, BVC, Relative, "50.json" ); */
/* CPU_TEST( 51, EOR, IndirectY, "51.json" ); */
/* CPU_TEST( 55, EOR, ZeroPageX, "55.json" ); */
/* CPU_TEST( 56, LSR, ZeroPageX, "56.json" ); */
/* CPU_TEST( 58, CLI, Implied, "58.json" ); */
/* CPU_TEST( 59, EOR, AbsoluteY, "59.json" ); */
/* CPU_TEST( 5D, EOR, AbsoluteX, "5d.json" ); */
/* CPU_TEST( 5E, LSR, AbsoluteX, "5e.json" ); */
/* CPU_TEST( 60, RTS, Implied, "60.json" ); */
/* CPU_TEST( 61, ADC, IndirectX, "61.json" ); */
/* CPU_TEST( 65, ADC, ZeroPage, "65.json" ); */
/* CPU_TEST( 66, ROR, ZeroPage, "66.json" ); */
/* CPU_TEST( 68, PLA, Implied, "68.json" ); */
/* CPU_TEST( 69, ADC, Immediate, "69.json" ); */
/* CPU_TEST( 6A, ROR, Accumulator, "6a.json" ); */
/* CPU_TEST( 6C, JMP, Indirect, "6c.json" ); */
/* CPU_TEST( 6D, ADC, Absolute, "6d.json" ); */
/* CPU_TEST( 6E, ROR, Absolute, "6e.json" ); */
/* CPU_TEST( 70, BVS, Relative, "70.json" ); */
/* CPU_TEST( 71, ADC, IndirectY, "71.json" ); */
/* CPU_TEST( 75, ADC, ZeroPageX, "75.json" ); */
/* CPU_TEST( 76, ROR, ZeroPageX, "76.json" ); */
/* CPU_TEST( 78, SEI, Implied, "78.json" ); */
/* CPU_TEST( 79, ADC, AbsoluteY, "79.json" ); */
/* CPU_TEST( 7D, ADC, AbsoluteX, "7d.json" ); */
/* CPU_TEST( 7E, ROR, AbsoluteX, "7e.json" ); */
/* CPU_TEST( 81, STA, IndirectX, "81.json" ); */
/* CPU_TEST( 84, STY, ZeroPage, "84.json" ); */
/* CPU_TEST( 85, STA, ZeroPage, "85.json" ); */
/* CPU_TEST( 86, STX, ZeroPage, "86.json" ); */
/* CPU_TEST( 88, DEY, Implied, "88.json" ); */
/* CPU_TEST( 8A, TXA, Implied, "8a.json" ); */
/* CPU_TEST( 8C, STY, Absolute, "8c.json" ); */
/* CPU_TEST( 8D, STA, Absolute, "8d.json" ); */
/* CPU_TEST( 8E, STX, Absolute, "8e.json" ); */
/* CPU_TEST( 90, BCC, Relative, "90.json" ); */
/* CPU_TEST( 91, STA, IndirectY, "91.json" ); */
/* CPU_TEST( 94, STY, ZeroPageX, "94.json" ); */
/* CPU_TEST( 95, STA, ZeroPageX, "95.json" ); */
/* CPU_TEST( 96, STX, ZeroPageY, "96.json" ); */
/* CPU_TEST( 98, TYA, Implied, "98.json" ); */
/* CPU_TEST( 99, STA, AbsoluteY, "99.json" ); */
/* CPU_TEST( 9A, TXS, Implied, "9a.json" ); */
/* CPU_TEST( 9D, STA, AbsoluteX, "9d.json" ); */
/* CPU_TEST( A0, LDY, Immediate, "a0.json" ); */
/* CPU_TEST( A1, LDA, IndirectX, "a1.json" ); */
/* CPU_TEST( A2, LDX, Immediate, "a2.json" ); */
/* CPU_TEST( A4, LDY, ZeroPage, "a4.json" ); */
/* CPU_TEST( A5, LDA, ZeroPage, "a5.json" ); */
/* CPU_TEST( A6, LDX, ZeroPage, "a6.json" ); */
/* CPU_TEST( A8, TAY, Implied, "a8.json" ); */
CPU_TEST( A9, LDA, Immediate, "a9.json" );
/* CPU_TEST( AA, TAX, Implied, "aa.json" ); */
/* CPU_TEST( AC, LDY, Absolute, "ac.json" ); */
/* CPU_TEST( AD, LDA, Absolute, "ad.json" ); */
/* CPU_TEST( AE, LDX, Absolute, "ae.json" ); */
/* CPU_TEST( B0, BCS, Relative, "b0.json" ); */
/* CPU_TEST( B1, LDA, IndirectY, "b1.json" ); */
/* CPU_TEST( B4, LDY, ZeroPageX, "b4.json" ); */
/* CPU_TEST( B5, LDA, ZeroPageX, "b5.json" ); */
/* CPU_TEST( B6, LDX, ZeroPageY, "b6.json" ); */
/* CPU_TEST( B8, CLV, Implied, "b8.json" ); */
/* CPU_TEST( B9, LDA, AbsoluteY, "b9.json" ); */
/* CPU_TEST( BA, TSX, Implied, "ba.json" ); */
/* CPU_TEST( BC, LDY, AbsoluteX, "bc.json" ); */
/* CPU_TEST( BD, LDA, AbsoluteX, "bd.json" ); */
/* CPU_TEST( BE, LDX, AbsoluteY, "be.json" ); */
/* CPU_TEST( C0, CPY, Immediate, "c0.json" ); */
/* CPU_TEST( C1, CMP, IndirectX, "c1.json" ); */
/* CPU_TEST( C4, CPY, ZeroPage, "c4.json" ); */
/* CPU_TEST( C5, CMP, ZeroPage, "c5.json" ); */
/* CPU_TEST( C6, DEC, ZeroPage, "c6.json" ); */
/* CPU_TEST( C8, INY, Implied, "c8.json" ); */
/* CPU_TEST( C9, CMP, Immediate, "c9.json" ); */
/* CPU_TEST( CA, DEX, Implied, "ca.json" ); */
/* CPU_TEST( CC, CPY, Absolute, "cc.json" ); */
/* CPU_TEST( CD, CMP, Absolute, "cd.json" ); */
/* CPU_TEST( CE, DEC, Absolute, "ce.json" ); */
/* CPU_TEST( D0, BNE, Relative, "d0.json" ); */
/* CPU_TEST( D1, CMP, IndirectY, "d1.json" ); */
/* CPU_TEST( D5, CMP, ZeroPageX, "d5.json" ); */
/* CPU_TEST( D6, DEC, ZeroPageX, "d6.json" ); */
/* CPU_TEST( D8, CLD, Implied, "d8.json" ); */
/* CPU_TEST( D9, CMP, AbsoluteY, "d9.json" ); */
/* CPU_TEST( DD, CMP, AbsoluteX, "dd.json" ); */
/* CPU_TEST( DE, DEC, AbsoluteX, "de.json" ); */
/* CPU_TEST( E0, CPX, Immediate, "e0.json" ); */
/* CPU_TEST( E1, SBC, IndirectX, "e1.json" ); */
/* CPU_TEST( E4, CPX, ZeroPage, "e4.json" ); */
/* CPU_TEST( E5, SBC, ZeroPage, "e5.json" ); */
/* CPU_TEST( E6, INC, ZeroPage, "e6.json" ); */
/* CPU_TEST( E8, INX, Implied, "e8.json" ); */
/* CPU_TEST( E9, SBC, Immediate, "e9.json" ); */
/* CPU_TEST( EA, NOP, Implied, "ea.json" ); */
/* CPU_TEST( EC, CPX, Absolute, "ec.json" ); */
/* CPU_TEST( ED, SBC, Absolute, "ed.json" ); */
/* CPU_TEST( EE, INC, Absolute, "ee.json" ); */
/* CPU_TEST( F0, BEQ, Relative, "f0.json" ); */
/* CPU_TEST( F1, SBC, IndirectY, "f1.json" ); */
/* CPU_TEST( F5, SBC, ZeroPageX, "f5.json" ); */
/* CPU_TEST( F6, INC, ZeroPageX, "f6.json" ); */
/* CPU_TEST( F8, SED, Implied, "f8.json" ); */
/* CPU_TEST( F9, SBC, AbsoluteY, "f9.json" ); */
/* CPU_TEST( FD, SBC, AbsoluteX, "fd.json" ); */
/* CPU_TEST( FE, INC, AbsoluteX, "fe.json" ); */

// -----------------------------------------------------------------------------
// -------------------------TEST CLASS METHODS ---------------------------------
// -----------------------------------------------------------------------------

void CPUTestFixture::RunTestCase( const json &testCase ) // NOLINT
{
    // Initialize CPU
    cpu.Reset();

    LoadStateFromJson( testCase, "initial" );
    std::string const initial_state = GetCPUStateString( testCase, "initial" );
    // Ensure loaded values match JSON values
    EXPECT_EQ( cpu.GetProgramCounter(), u16( testCase["initial"]["pc"] ) );
    EXPECT_EQ( cpu.GetAccumulator(), testCase["initial"]["a"] );
    EXPECT_EQ( cpu.GetXRegister(), testCase["initial"]["x"] );
    EXPECT_EQ( cpu.GetYRegister(), testCase["initial"]["y"] );
    EXPECT_EQ( cpu.GetStackPointer(), testCase["initial"]["s"] );
    EXPECT_EQ( cpu.GetStatusRegister(), testCase["initial"]["p"] );

    for ( const auto &ram_entry : testCase["initial"]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  value = ram_entry[1];
        EXPECT_EQ( cpu.Read( address ), value );
    }

    // Fetch, decode, execute
    cpu.Tick();

    // Check final state
    bool               test_failed = false; // Track if any test has failed
    std::ostringstream error_messages;      // Accumulate error messages
                                            //
    if ( cpu.GetProgramCounter() != u16( testCase["final"]["pc"] ) )
    {
        test_failed = true;
        error_messages << "PC ";
    }
    if ( cpu.GetAccumulator() != u8( testCase["final"]["a"] ) )
    {
        test_failed = true;
        error_messages << "A ";
    }
    if ( cpu.GetXRegister() != u8( testCase["final"]["x"] ) )
    {
        test_failed = true;
        error_messages << "X ";
    }
    if ( cpu.GetYRegister() != u8( testCase["final"]["y"] ) )
    {
        test_failed = true;
        error_messages << "Y ";
    }
    if ( cpu.GetStackPointer() != u8( testCase["final"]["s"] ) )
    {
        test_failed = true;
        error_messages << "S ";
    }
    if ( cpu.GetStatusRegister() != u8( testCase["final"]["p"] ) )
    {
        test_failed = true;
        error_messages << "P ";
    }
    if ( cpu.GetCycles() != testCase["cycles"].size() )
    {
        test_failed = true;
        error_messages << "Cycle count ";
    }

    for ( const auto &ram_entry : testCase["final"]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  expected_value = ram_entry[1];
        uint8_t const  actual_value = cpu.Read( address );
        if ( actual_value != expected_value )
        {
            test_failed = true;
            error_messages << "RAM ";
        }
    }

    std::string const final_state = GetCPUStateString( testCase, "final" );
    // print initial and final state if there are any failures
    if ( test_failed )
    {
        std::cout << "Test Case: " << testCase["name"] << '\n';
        std::cout << "Failed: " << error_messages.str() << '\n';
        std::cout << initial_state << '\n';
        std::cout << final_state << '\n';
        std::cout << '\n';
        FAIL();
    }
}

void CPUTestFixture::LoadStateFromJson( const json &jsonData, const std::string &state )
{
    /*
     This function loads the CPU state from json data.
     args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */
    cpu.SetProgramCounter( jsonData[state]["pc"] );
    cpu.SetAccumulator( jsonData[state]["a"] );
    cpu.SetXRegister( jsonData[state]["x"] );
    cpu.SetYRegister( jsonData[state]["y"] );
    cpu.SetStackPointer( jsonData[state]["s"] );
    cpu.SetStatusRegister( jsonData[state]["p"] );

    // Load memory state from JSON
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  value = ram_entry[1];
        cpu.Write( address, value );
    }
}

std::string CPUTestFixture::GetCPUStateString( const json &jsonData, const std::string &state )
{
    /*
    This function provides formatted output for expected vs. actual CPU state values,
    based on provided json data and actual CPU state.
    Args:
      jsonData: JSON data returned by extractTestsFromJson
      state: "initial" or "final"
    */

    // Expected values
    u16 const expected_pc = u16( jsonData[state]["pc"] );
    u8 const  expected_a = jsonData[state]["a"];
    u8 const  expected_x = jsonData[state]["x"];
    u8 const  expected_y = jsonData[state]["y"];
    u8 const  expected_s = jsonData[state]["s"];
    u8 const  expected_p = jsonData[state]["p"];
    u64 const expected_cycles = jsonData["cycles"].size();

    // Actual values
    u16 const actual_pc = cpu.GetProgramCounter();
    u8 const  actual_a = cpu.GetAccumulator();
    u8 const  actual_x = cpu.GetXRegister();
    u8 const  actual_y = cpu.GetYRegister();
    u8 const  actual_s = cpu.GetStackPointer();
    u8 const  actual_p = cpu.GetStatusRegister();
    u64 const actual_cycles = cpu.GetCycles();

    // Column Widths
    const int label_width = 8;
    const int value_width = 14;

    // Use ostringstream to collect output
    std::ostringstream output;

    // Print header
    output << "----------" << state << " State----------" << '\n';
    output << std::left << std::setw( label_width ) << "" << std::setw( value_width ) << "EXPECTED"
           << std::setw( value_width ) << "ACTUAL" << '\n';

    // Function to format and print a line
    auto print_line = [&]( const std::string &label, uint64_t expected, uint64_t actual )
    {
        auto to_hex_decimal_string = []( uint64_t value, int width )
        {
            std::stringstream str_stream;
            str_stream << std::hex << std::uppercase << std::setw( width ) << std::setfill( '0' )
                       << value << " (" << std::dec << value << ")";
            return str_stream.str();
        };

        int width = 4;
        if ( expected > 0xFFFF || actual > 0xFFFF )
        {
            width = 8;
        }
        else if ( expected > 0xFF || actual > 0xFF )
        {
            width = 4;
        }
        else
        {
            width = 2;
        }

        output << std::left << std::setw( label_width ) << label;
        output << std::setw( value_width ) << to_hex_decimal_string( expected, width );
        output << std::setw( value_width ) << to_hex_decimal_string( actual, width ) << '\n';
    };

    // Print registers
    print_line( "pc:", expected_pc, actual_pc );
    print_line( "s:", expected_s, actual_s );
    print_line( "a:", expected_a, actual_a );
    print_line( "x:", expected_x, actual_x );
    print_line( "y:", expected_y, actual_y );
    print_line( "p:", expected_p, actual_p );

    if ( state == "final" )
    {
        output << std::left << std::setw( label_width ) << "cycles:";
        output << std::setw( value_width ) << expected_cycles;
        output << std::setw( value_width ) << actual_cycles << '\n';
    }

    // Blank line and RAM header
    output << '\n' << "RAM" << '\n';

    // Print RAM entries
    for ( const auto &ram_entry : jsonData[state]["ram"] )
    {
        uint16_t const address = ram_entry[0];
        uint8_t const  expected_value = ram_entry[1];
        uint8_t const  actual_value = cpu.Read( address );

        // Helper lambda to format values as "HEX (DECIMAL)"
        auto format_value = []( uint8_t value )
        {
            std::ostringstream oss;
            oss << std::hex << std::uppercase << std::setw( 2 ) << std::setfill( '0' )
                << static_cast<int>( value ) << " (" << std::dec << static_cast<int>( value )
                << ")";
            return oss.str();
        };

        // Format address as hex only (no decimal for addresses)
        std::ostringstream address_stream;
        address_stream << std::hex << std::setw( 4 ) << std::setfill( '0' ) << address;

        // Print formatted output
        output << std::left << std::setw( label_width ) << address_stream.str();
        output << std::setw( value_width ) << format_value( expected_value );
        output << std::setw( value_width ) << format_value( actual_value ) << '\n';
    }

    output << "--------------------------------" << '\n';
    output << '\n';

    // Return the accumulated string
    return output.str();
}

// -----------------------------------------------------------------------------
// --------------------------- GENERAL HELPERS ---------------------------------
//               Helpers that don't depend on the CPU class
// -----------------------------------------------------------------------------

auto extractTestsFromJson( const std::string &path ) -> json
// Extracts test cases from a JSON file and returns them as a JSON object, with the
// help of the nlohmann::json library.
{
    std::ifstream json_file( path );
    if ( !json_file.is_open() )
    {
        throw std::runtime_error( "Could not open test file: " + path );
    }
    json test_cases;
    json_file >> test_cases;

    if ( !test_cases.is_array() )
    {
        throw std::runtime_error( "Expected an array of test cases in JSON file" );
    }
    return test_cases;
}

void printTestStartMsg( const std::string &testName )
{
    std::cout << '\n';
    std::cout << "---------- " << testName << " Tests ---------" << '\n';
}
void printTestEndMsg( const std::string &testName )
{
    std::cout << "---------- " << testName << " Tests Complete ---------" << '\n';
    std::cout << '\n';
}

// -----------------------------------------------------------------------------
// -------------------------------- MAIN ---------------------------------------
// -----------------------------------------------------------------------------
auto main( int argc, char **argv ) -> int
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
