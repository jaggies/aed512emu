#ifndef DIS6502_H
#define DIS6502_H

#include <string>
#include <tuple>
#include <cstdint>
#include <functional>

std::tuple<int, std::string> disassemble_6502(int address, const std::function<uint8_t(int offset)>& read);

#endif // DIS6502_H
