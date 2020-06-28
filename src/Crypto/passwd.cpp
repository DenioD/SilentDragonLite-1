#include "passwd.h"

void PASSWD::show_hex_buff(unsigned char buf[])
{
    int i;
    for (uint8_t i=0; i < crypto_secretstream_xchacha20poly1305_KEYBYTES; i++) 
        printf("%02X ", buf[i]);
    printf("\n");
}

const unsigned char* PASSWD::key(QString password)
{
   
    int length = password.length();
     
    char *sequence = NULL;
    sequence = new char[length+1];
    strncpy(sequence, password.toLocal8Bit(), length +1);

    #define MESSAGE ((const unsigned char *) sequence)
    #define MESSAGE_LEN length
  
    unsigned char hash[crypto_secretstream_xchacha20poly1305_KEYBYTES];

    crypto_hash_sha256(hash, MESSAGE, MESSAGE_LEN);

    qDebug()<<"Generating SaltHash from password: " <<sequence;


     /////////we use the Hash of the Password as Salt, not perfect but still a good solution.

    #define PASSWORD sequence
    #define KEY_LEN crypto_box_SEEDBYTES

    unsigned char key[KEY_LEN];

    if (crypto_pwhash
    (key, sizeof key, PASSWORD, strlen(PASSWORD), hash,
     crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE,
     crypto_pwhash_ALG_DEFAULT) != 0) {
    /* out of memory */
}

    qDebug()<<"Generating cryptographic key from password: " <<sequence;

    
  //  crypto_generichash(blacke2hash, sizeof hash, MESSAGE, MESSAGE_LEN, NULL, 0);

    //for(uint8_t i = 0; i < crypto_secretstream_xchacha20poly1305_KEYBYTES/2; i++)
      //  hash[i] = blacke2hash[i];

  //  for(uint8_t i = crypto_secretstream_xchacha20poly1305_KEYBYTES/2; i < crypto_secretstream_xchacha20poly1305_KEYBYTES; i++)
    //    hash[i] = sha256hash[i];

   // delete[] sha256hash;
    //delete[] blacke2hash;
    qDebug()<<"secret key generated:\n";
    PASSWD::show_hex_buff(key);
    return key;
}