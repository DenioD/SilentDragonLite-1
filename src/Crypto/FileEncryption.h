#ifndef FILEENCRYPTION_H
#define FILEENCRYPTION_H
#include <stdio.h>
#include <sodium.h>
#include <QString>
#include <fstream>

#define FILEENCRYPTION_CHUNK_SIZE 4096

class FileEncryption
{
    private:
        inline static bool exists (const std::string& name) {
            std::ifstream f(name.c_str());
            return f.good();
        }
    public:
        static void showConfig();
        static int encrypt(QString target_file, QString source_file, const unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES]);
        static int decrypt(QString target_file, QString source_file, const unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES]);
};


#endif