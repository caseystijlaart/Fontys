/**
 * @file parser.cpp
 * @author Casey Stijlaart (casey.stijlaart@hotmail.com)
 * @brief Implementation of the parser class (parser.hpp)
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "parser.hpp"

#include <iomanip>
#include <iostream>

Parser::Parser() : currentState(&Parser::StartBytes), payload(nullptr) {}

uint8_t Parser::Parse(uint8_t* data, size_t buffer_size) {
  if (data == nullptr) {
    return 1;
  }

  while (buffer_size > 0) {
    uint8_t result = (this->*currentState)(data, buffer_size);
    if (result != 0) {
      return result;
    }
  }

  return 0;
}

uint8_t Parser::StartBytes(uint8_t* data, size_t& buffer_size) {
  if (data == nullptr) {
    return 1;
  }

  static size_t start_byte_index = 0;
  while (buffer_size > 0 && start_byte_index < Msg_Structure::NUM_START_BYTES) {
    if (data[0] == Msg_Structure::START_BYTES[start_byte_index]) {
      start_byte_index++;
      std::memmove(data, data + 1, buffer_size--);
    } else {
      start_byte_index = 0;
      return 1;
    }
  }

  if (start_byte_index == Msg_Structure::NUM_START_BYTES) {
    start_byte_index = 0;
    currentState = &Parser::PayloadSize;
  }

  return 0;
}

uint8_t Parser::PayloadSize(uint8_t* data, size_t& buffer_size) {
  if (data == nullptr) {
    return 1;
  }

  if (buffer_size > 0) {
    payload_size = data[0];
    std::memmove(data, data + 1, buffer_size--);
    currentState = &Parser::CommandId;
  }
  return 0;
}

uint8_t Parser::CommandId(uint8_t* data, size_t& buffer_size) {
  if (data == nullptr) {
    return 1;
  }

  if (buffer_size > 0) {
    cur_size = 0;
    if (payload == nullptr) {
      payload = new uint8_t[payload_size];
    }
    payload[cur_size++] = data[0];
    std::memmove(data, data + 1, buffer_size--);
    currentState = &Parser::Payload;
  }
  return 0;
}

uint8_t Parser::Payload(uint8_t* data, size_t& buffer_size) {
  if (data == nullptr) {
    return 1;
  }

  while (buffer_size > 0 && cur_size < payload_size) {
    payload[cur_size++] = data[0];
    std::memmove(data, data + 1, buffer_size--);
  }

  if (cur_size == payload_size) {
    currentState = &Parser::Crc;
  }

  return 0;
}

uint8_t Parser::Crc(uint8_t* data, size_t& buffer_size) {
  if (data == nullptr) {
    return 1;
  }
  if (buffer_size > 0) {
    uint8_t received_crc = data[0];
    uint8_t calculated_crc = CalculateCrc(payload, payload_size);
    if (received_crc != calculated_crc) {
      delete[] payload;
      payload = nullptr;
      return 1;
    }

    parsed_state = true;
    currentState = &Parser::StartBytes;
    std::memmove(data, data + 1, buffer_size--);
  }

  return 0;
}

uint8_t Parser::CalculateCrc(const uint8_t* data, size_t size) {
  if (data == nullptr) {
    return 1;
  }

  uint8_t crc = 0x00;

  while (size--) {
    crc ^= *data++;
    for (uint8_t i = 0; i < 8; ++i) {
      crc = (crc & 0x80) ? (crc << 1) ^ Msg_Structure::CRC_8_VALUE : (crc << 1);
    }
  }

  return crc;
}

bool Parser::GetParsedState() { return parsed_state; }

uint8_t* Parser::GetPayload() {
  if (parsed_state && payload != nullptr) {
    parsed_state = false;
    uint8_t* temp_payload = payload;
    payload = nullptr;
    payload_size = 0;
    return temp_payload;
  }
  return nullptr;
}
