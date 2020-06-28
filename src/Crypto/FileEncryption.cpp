#include "FileEncryption.h"

void FileEncryption::showConfig()
{
    qInfo() << FILEENCRYPTION_CHUNK_SIZE;
}

int FileEncryption::encrypt(QString target_file, QString source_file, const unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES])
{
    unsigned char  plain_data[FILEENCRYPTION_CHUNK_SIZE];
    unsigned char  cipher_data[FILEENCRYPTION_CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char  header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state state;
    FILE          *target, *source;
    unsigned long long cipher_len;
    size_t         rlen;
    int            eof;
    unsigned char  tag;

    if(!FileEncryption::exists(source_file.toStdString()))
    {
        qDebug() << "File not exits" << source_file;
        return -1;
    }

    source = fopen(source_file.toStdString().c_str(), "rb");
    target = fopen(target_file.toStdString().c_str(), "wb");
    crypto_secretstream_xchacha20poly1305_init_push(&state, header, key);
    fwrite(header, 1, sizeof header, target);
    do 
    {
        rlen = fread(plain_data, 1, sizeof plain_data, source);
        eof = feof(source);
        tag = eof ? crypto_secretstream_xchacha20poly1305_TAG_FINAL : 0;
        crypto_secretstream_xchacha20poly1305_push(
            &state, 
            cipher_data, 
            &cipher_len, 
            plain_data, 
            rlen,
            NULL, 
            0, 
            tag
        );

        fwrite(cipher_data, 1, (size_t) cipher_len, target);
    } 
    while (! eof);

    fclose(target);
    fclose(source);
    return 0;
}

int FileEncryption::decrypt(QString target_file, QString source_file, const unsigned char key[crypto_secretstream_xchacha20poly1305_KEYBYTES])
{
    unsigned char  buf_in[FILEENCRYPTION_CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES];
    unsigned char  buf_out[FILEENCRYPTION_CHUNK_SIZE];
    unsigned char  header[crypto_secretstream_xchacha20poly1305_HEADERBYTES];
    crypto_secretstream_xchacha20poly1305_state st;
    FILE          *fp_t, *fp_s;
    unsigned long long out_len;
    size_t         rlen;
    int            eof;
    int            ret = -1;
    unsigned char  tag;

    if(!FileEncryption::exists(source_file.toStdString()))
    {
        qDebug() << "File not exits" << source_file;
        return -1;
    }

    fp_s = fopen(source_file.toStdString().c_str(), "rb");
    fp_t = fopen(target_file.toStdString().c_str(), "wb");
    fread(header, 1, sizeof header, fp_s);
    if (crypto_secretstream_xchacha20poly1305_init_pull(&st, header, key) != 0) 
    {
        goto ret; /* incomplete header */
    }

    do 
    {
        rlen = fread(buf_in, 1, sizeof buf_in, fp_s);
        eof = feof(fp_s);
        if (crypto_secretstream_xchacha20poly1305_pull(
            &st, 
            buf_out, 
            &out_len, 
            &tag,
            buf_in, 
            rlen, 
            NULL, 
            0
            ) != 0) 
                                                       {
            goto ret; /* corrupted chunk */
        }

        if (tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL && ! eof) 
        {
            goto ret; /* premature end (end of file reached before the end of the stream) */
        }

        fwrite(buf_out, 1, (size_t) out_len, fp_t);
    }
    while (! eof);
    ret = 0;

ret:
    fclose(fp_t);
    fclose(fp_s);
    return ret;
}