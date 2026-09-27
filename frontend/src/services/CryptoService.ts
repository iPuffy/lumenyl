export interface KeyPair
{
    public_key: JsonWebKey;
    private_key: CryptoKey;
}

export interface EncryptedMessage
{
    iv: string;
    ciphertext: string;
}

function array_buffer_to_base64(buffer: ArrayBuffer): string
{
    const bytes = new Uint8Array(buffer);

    let binary = "";

    for (const byte of bytes)
    {
        binary += String.fromCharCode(byte);
    }

    return btoa(binary);
}

function base64_to_array_buffer(base64: string): ArrayBuffer
{
    const binary = atob(base64);

    const bytes = new Uint8Array(binary.length);

    for (let i = 0; i < binary.length; i++)
    {
        bytes[i] = binary.charCodeAt(i);
    }

    return bytes.buffer;
}

export async function generate_key_pair(): Promise<KeyPair>
{
    const key_pair =
        await crypto.subtle.generateKey(
            {
                name: "ECDH",
                namedCurve: "P-256"
            },
            true,
            ["deriveKey"]
        );

    const public_key =
        await crypto.subtle.exportKey(
            "jwk",
            key_pair.publicKey
        );

    return {
        public_key,
        private_key: key_pair.privateKey
    };
}

export async function derive_shared_key
    (
        private_key: CryptoKey,
        other_public_key: JsonWebKey
    ): Promise<CryptoKey>
{
    const imported_public_key =
        await crypto.subtle.importKey(
            "jwk",
            other_public_key,
            {
                name: "ECDH",
                namedCurve: "P-256"
            },
            false,
            []
        );

    return crypto.subtle.deriveKey(
        {
            name: "ECDH",
            public: imported_public_key
        },
        private_key,
        {
            name: "AES-GCM",
            length: 256
        },
        false,
        ["encrypt", "decrypt"]
    );
}

export async function encrypt_message
    (
        message: string,
        shared_key: CryptoKey
    ): Promise<EncryptedMessage>
{
    const iv =
        crypto.getRandomValues(
            new Uint8Array(12)
        );

    const plaintext =
        new TextEncoder().encode(message);

    const ciphertext =
        await crypto.subtle.encrypt(
            {
                name: "AES-GCM",
                iv
            },
            shared_key,
            plaintext
        );

    return {
        iv: array_buffer_to_base64(iv.buffer),
        ciphertext: array_buffer_to_base64(ciphertext)
    };
}

export async function decrypt_message
    (
        encrypted_message: EncryptedMessage,
        shared_key: CryptoKey
    ): Promise<string>
{
    const iv =
        new Uint8Array(
            base64_to_array_buffer(
                encrypted_message.iv
            )
        );

    const ciphertext =
        base64_to_array_buffer(
            encrypted_message.ciphertext
        );

    const plaintext =
        await crypto.subtle.decrypt(
            {
                name: "AES-GCM",
                iv
            },
            shared_key,
            ciphertext
        );

    return new TextDecoder().decode(plaintext);
}