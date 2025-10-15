<?php

class EncryptionService
{
    private const CIPHER = 'aes-256-gcm';
    private const KEY_LENGTH = 32;
    private const TAG_LENGTH = 16;

    private string $encryptionKey;

    public function __construct(?string $key = null)
    {
        if ($key === null) {
            // Essayer de lire depuis le fichier encryption.key
            $keyFile = __DIR__ . '/../encryption.key';

            if (file_exists($keyFile)) {
                $key = trim(file_get_contents($keyFile));
            } elseif (isset($_ENV['ENCRYPTION_KEY'])) {
                // Fallback sur la variable d'environnement si le fichier n'existe pas
                $key = $_ENV['ENCRYPTION_KEY'];
            } else {
                throw new Exception('Encryption key not found. Please create encryption.key file or set ENCRYPTION_KEY environment variable.');
            }
        }

        // Convertir la clé hexadécimale en binaire si nécessaire
        if (strlen($key) === 64 && ctype_xdigit($key)) {
            // Clé hexadécimale de 64 caractères (256 bits)
            $key = hex2bin($key);
        } elseif (strlen($key) !== self::KEY_LENGTH) {
            // Hasher la clé si elle n'a pas la bonne longueur
            $key = hash('sha256', $key, true);
        }

        $this->encryptionKey = $key;
    }

    public function encrypt(string $data): string
    {
        $iv = random_bytes(openssl_cipher_iv_length(self::CIPHER));

        $tag = '';
        $encrypted = openssl_encrypt(
            $data,
            self::CIPHER,
            $this->encryptionKey,
            OPENSSL_RAW_DATA,
            $iv,
            $tag,
            '',
            self::TAG_LENGTH
        );

        if ($encrypted === false) {
            throw new Exception('Encryption failed');
        }

        return $iv . $tag . $encrypted;
    }

    public function decrypt(string $encryptedData): string
    {
        $ivLength = openssl_cipher_iv_length(self::CIPHER);

        if (strlen($encryptedData) < $ivLength + self::TAG_LENGTH) {
            throw new Exception('Invalid encrypted data: too short');
        }

        $iv = substr($encryptedData, 0, $ivLength);
        $tag = substr($encryptedData, $ivLength, self::TAG_LENGTH);
        $ciphertext = substr($encryptedData, $ivLength + self::TAG_LENGTH);

        $decrypted = openssl_decrypt(
            $ciphertext,
            self::CIPHER,
            $this->encryptionKey,
            OPENSSL_RAW_DATA,
            $iv,
            $tag
        );

        if ($decrypted === false) {
            throw new Exception('Decryption failed: invalid key or corrupted data');
        }

        return $decrypted;
    }

    public static function generateKey(): string
    {
        return bin2hex(random_bytes(self::KEY_LENGTH));
    }

    public function encryptFile(string $inputPath, string $outputPath): bool
    {
        if (!file_exists($inputPath)) {
            throw new Exception("Input file not found: $inputPath");
        }

        $data = file_get_contents($inputPath);
        if ($data === false) {
            throw new Exception("Failed to read input file: $inputPath");
        }

        $encrypted = $this->encrypt($data);

        $result = file_put_contents($outputPath, $encrypted);
        if ($result === false) {
            throw new Exception("Failed to write encrypted file: $outputPath");
        }

        return true;
    }

    public function decryptFile(string $inputPath, string $outputPath): bool
    {
        if (!file_exists($inputPath)) {
            throw new Exception("Encrypted file not found: $inputPath");
        }

        $encryptedData = file_get_contents($inputPath);
        if ($encryptedData === false) {
            throw new Exception("Failed to read encrypted file: $inputPath");
        }

        $decrypted = $this->decrypt($encryptedData);

        $result = file_put_contents($outputPath, $decrypted);
        if ($result === false) {
            throw new Exception("Failed to write decrypted file: $outputPath");
        }

        return true;
    }
}
