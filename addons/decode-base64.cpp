#include <stdexcept>
#include <string>
#include <vector>
#include <fstream>
#include <napi.h>

class Base64Decoder {
private:
    static const std::string base64Chars;
    static const std::vector<int> decodingTable;

    // Initialize the decoding table
    static std::vector<int> initDecodingTable() {
        std::vector<int> table(256, -1);
        for (size_t i = 0; i < base64Chars.size(); ++i) {
            table[base64Chars[i]] = i;
        }
        return table;
    }

public:
    static std::string decode(const std::string& encoded) {
        if (encoded.empty()) return "";

        if (encoded.length() % 4 != 0) {
            throw std::invalid_argument("Invalid Base64 string length");
        }

        validateBase64(encoded);

        size_t padding = countPadding(encoded);
        size_t length = (encoded.size() * 3) / 4 - padding;
        std::string decoded;
        decoded.resize(length);

        size_t j = 0;
        uint32_t temp = 0;
        size_t chars_processed = 0;

        for (size_t i = 0; i < encoded.length(); ++i) {
            if (encoded[i] == '=') break;

            temp = (temp << 6) + decodingTable[encoded[i]];
            chars_processed++;

            if (chars_processed == 4) {
                if (j + 3 <= length) {
                    decoded[j++] = (temp >> 16) & 0xFF;
                    decoded[j++] = (temp >> 8) & 0xFF;
                    decoded[j++] = temp & 0xFF;
                }
                chars_processed = 0;
                temp = 0;
            }
        }

        handlePadding(temp, padding, decoded, j, length);

        return decoded;
    }

private:
    static void validateBase64(const std::string& encoded) {
        for (size_t i = 0; i < encoded.length(); ++i) {
            if (encoded[i] == '=') {
                if (i < encoded.length() - 2) {
                    throw std::invalid_argument("Invalid padding position");
                }
            } else if (decodingTable[encoded[i]] == -1) {
                throw std::invalid_argument("Invalid Base64 character found");
            }
        }
    }

    static size_t countPadding(const std::string& encoded) {
        size_t padding = 0;
        if (encoded.size() >= 2 && encoded[encoded.size() - 1] == '=') {
            padding = (encoded[encoded.size() - 2] == '=') ? 2 : 1;
        }
        return padding;
    }

    static void handlePadding(uint32_t temp, size_t padding, std::string& decoded, size_t& j, size_t length) {
        if (padding > 0) {
            temp <<= 6 * padding;
            if (padding == 1 && j + 2 <= length) {
                decoded[j++] = (temp >> 16) & 0xFF;
                decoded[j++] = (temp >> 8) & 0xFF;
            } else if (padding == 2 && j + 1 <= length) {
                decoded[j++] = (temp >> 16) & 0xFF;
            }
        }
    }
};

// Static member initialization
const std::string Base64Decoder::base64Chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

const std::vector<int> Base64Decoder::decodingTable = Base64Decoder::initDecodingTable();

// Function to decode a Base64 file
Napi::String DecodeFile(const Napi::CallbackInfo &info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString()) {
        Napi::TypeError::New(env, "String arguments expected for input and output paths")
            .ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    std::string inputFile = info[0].As<Napi::String>();
    std::string outputFile = info[1].As<Napi::String>();

    // Read the Base64 file
    std::ifstream inFile(inputFile, std::ios::in);
    if (!inFile.is_open()) {
        Napi::Error::New(env, "Failed to open input file").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    std::string encodedContent((std::istreambuf_iterator<char>(inFile)),
                               std::istreambuf_iterator<char>());
    inFile.close();

    // Decode the Base64 content
    std::string decodedContent = Base64Decoder::decode(encodedContent);

    // Write the decoded content to the output file
    std::ofstream outFile(outputFile, std::ios::out | std::ios::binary);
    if (!outFile.is_open()) {
        Napi::Error::New(env, "Failed to create output file").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    outFile.write(decodedContent.data(), decodedContent.size());
    outFile.close();

    return Napi::String::New(env, "File decoded successfully!");
}

// Register the function
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "decodeFile"), Napi::Function::New(env, DecodeFile));
    return exports;
}

NODE_API_MODULE(base64_decode, Init)
