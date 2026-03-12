#include "toml.h"

#include <math.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <assert.h>

#include "commons.h"
#include "../logger/Logger.h"

using namespace toml;

// ========== Section ==========

Section::Section(std::string name) : name(name) {
}

void Section::add(std::string name, Field field) {
    if (fields.find(name) != fields.end()) {
        LOG_ERROR("Field duplication");
        throw std::runtime_error("Field duplication");
    }
    fields[name] = field;
    keyOrder.push_back(name);
}

// Публичные перегрузки add, каждая создаёт Field нужного типа.

void Section::add(std::string name, bool* ptr) {
    add(name, {fieldtype::ftbool, ptr});
}

void Section::add(std::string name, int* ptr) {
    add(name, {fieldtype::ftint, ptr});
}

void Section::add(std::string name, uint* ptr) {
    add(name, {fieldtype::ftuint, ptr});
}

void Section::add(std::string name, float* ptr) {
    add(name, {fieldtype::ftfloat, ptr});
}

void Section::add(std::string name, std::string* ptr) {
    add(name, {fieldtype::ftstring, ptr});
}

std::string Section::getName() const {
    return name;
}

const Field* Section::field(std::string name) const {
    auto found = fields.find(name);
    if (found == fields.end()) return nullptr;
    return &found->second;
}

const std::vector<std::string>& Section::keys() const {
    return keyOrder;
}

// ========== Wrapper ==========

Wrapper::~Wrapper() {
    for (auto entry : sections) {
        delete entry.second;
    }
}

Section& Wrapper::add(std::string name) {
    if (sections.find(name) != sections.end()) {
        LOG_ERROR("Section duplication");
        throw std::runtime_error("Section duplication");
    }
    Section* section = new Section(name);
    sections[name] = section;
    keyOrder.push_back(name);
    return *section;
}

Section* Wrapper::section(std::string name) {
    auto found = sections.find(name);
    if (found == sections.end()) return nullptr;
    return found->second;
}

std::string Wrapper::write() const {
    std::stringstream ss;
    for (std::string key : keyOrder) {
        const Section* section = sections.at(key);
        ss << "[" << key << "]\n";
        for (const std::string& key : section->keys()) {
            ss << key << " = ";
            const Field* field = section->field(key);
            assert(field != nullptr);
            switch (field->type) {
                case fieldtype::ftbool:
                    ss << (*((bool*)field->ptr) ? "true" : "false");
                    break;
                case fieldtype::ftint: ss << *((int*)field->ptr); break;
                case fieldtype::ftuint: ss << *((uint*)field->ptr); break;
                case fieldtype::ftfloat: ss << *((float*)field->ptr); break;
                case fieldtype::ftstring: 
                    ss << escape_string(*((const std::string*)field->ptr)); 
                    break;
            }
            ss << "\n";
        }
        ss << "\n";
    }
    return ss.str();
}

// ========== Reader ==========

Reader::Reader(Wrapper* wrapper, std::string file, std::string source) : BasicParser(file, source), wrapper(wrapper) {
}

void Reader::skipWhitespace() {
    // Переопределяем skipWhitespace, чтобы пропускать также комментарии (# до конца строки).
    BasicParser::skipWhitespace();
    if (hasNext() && source[pos] == '#') {
        skipLine();
        if (hasNext() && is_whitespace(peek())) skipWhitespace();
    }
}

void Reader::read() {
    skipWhitespace();
    if (!hasNext()) return;
    readSection(nullptr);
}

// Вспомогательная функция для проверки, является ли тип числовым.
inline bool is_numeric_type(fieldtype type) {
    return type == fieldtype::ftint || type == fieldtype::ftfloat;
}

void Section::set(std::string name, double value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        LOG_WARN("Unknown key: '{}'", name);
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = fabs(value) > 0.0; break;
        case fieldtype::ftint: *(int*)(field->ptr) = value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = value; break;
        case fieldtype::ftstring: *(std::string*)(field->ptr) = std::to_string(value); break;
        default:
            LOG_ERROR("Type error for key '{}'", name);
        }
    }
}

void Section::set(std::string name, bool value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        LOG_WARN("Unknown key: '{}'", name);
    } else {
        switch (field->type) {
        case fieldtype::ftbool: *(bool*)(field->ptr) = value; break;
        case fieldtype::ftint: *(int*)(field->ptr) = (int)value; break;
        case fieldtype::ftuint: *(uint*)(field->ptr) = (uint)value; break;
        case fieldtype::ftfloat: *(float*)(field->ptr) = (float)value; break;
        case fieldtype::ftstring: *(std::string*)(field->ptr) = value ? "true" : "false"; break;
        default:
            LOG_ERROR("Type error for key '{}'", name);
        }
    }
}

void Section::set(std::string name, std::string value) {
    const Field* field = this->field(name);
    if (field == nullptr) {
        LOG_WARN("Unknown key: '{}'", name);
    } else {
        switch (field->type) {
        case fieldtype::ftstring: *(std::string*)(field->ptr) = value; break;
        default:
            LOG_ERROR("Type error for key '{}'", name);
        }
    }
}

void Reader::readSection(Section* section /*nullable*/) {
    while (hasNext()) {
        skipWhitespace();
        if (!hasNext()) break;
        char c = nextChar();
        if (c == '[') {
            // Начало новой секции: читаем имя, затем ожидаем закрывающую ']'
            std::string name = parseName();
            Section* section = wrapper->section(name);
            pos++;
            readSection(section);
            return;
        }
        // Если не '[', то это ключ (или конец файла).
        pos--;
        std::string name = parseName(); // читаем имя ключа
        expect('=');
        c = peek();
        if (is_digit(c)) {
            // Число без знака
            number_u num;
            if (parseNumber(1, num) && section) section->set(name, (double)num.ival);
            else if (section) section->set(name, num.fval);
        } else if (c == '-' || c == '+') {
            // Число со знаком
            int sign = c == '-' ? -1 : 1;
            pos++;
            number_u num;
            if (parseNumber(sign, num) && section) section->set(name, (double)num.ival);
            else if (section) section->set(name, num.fval);
        } else if (is_identifier_start(c)) {
            // Идентификатор: возможно true/false/inf/nan
            std::string identifier = parseName();
            if (identifier == "true" || identifier == "false") {
                bool flag = identifier == "true";
                if (section) section->set(name, flag);
            } else if (identifier == "inf") {
                if (section) section->set(name, INFINITY);
            } else if (identifier == "nan") {
                if (section) section->set(name, NAN);
            }
        } else if (c == '"' || c == '\'') {
            // Строка
            pos++; // пропускаем открывающую кавычку
            std::string str = parseString(c); // читаем строку до закрывающей кавычки
            if (section) section->set(name, str);
        } else {
            LOG_ERROR("Feature is not supported", name);
            throw error("Feature is not supported");
        }
        expectNewLine(); // после значения ожидается перевод строки или конец файла
    }
}
