import { useEffect, useRef } from "react";

import
    {
        generate_key_pair,
        derive_shared_key,
        encrypt_message,
        decrypt_message
    } from "../services/CryptoService";

interface UseChatSocketProps
{
    on_message: (message: string) => void;
}

function useChatSocket
    (
        {
            on_message
        }: UseChatSocketProps
    )
{
    const socket = useRef<WebSocket | null>(null);

    const pending_messages =
        useRef<string[]>([]);

    const on_message_ref =
        useRef(on_message);

    const private_key =
        useRef<CryptoKey | null>(null);

    const shared_key =
        useRef<CryptoKey | null>(null);

    useEffect(() =>
    {
        on_message_ref.current = on_message;
    }, [on_message]);

    useEffect(() =>
    {
        let cancelled = false;

        async function connect()
        {
            const key_pair =
                await generate_key_pair();

            if (cancelled)
            {
                return;
            }

            private_key.current =
                key_pair.private_key;

            const protocol =
                window.location.protocol === "https:"
                    ? "wss:"
                    : "ws:";

            const socket_url =
                `${protocol}//${window.location.host}/chat`;

            console.log("Connecting to:", socket_url);

            const new_socket =
                new WebSocket(socket_url);

            socket.current = new_socket;

            new_socket.onopen = () =>
            {
                console.log("WebSocket connected");

                new_socket.send(
                    JSON.stringify({
                        type: "key_exchange",
                        public_key: key_pair.public_key
                    })
                );

                for (
                    const message of pending_messages.current
                )
                {
                    new_socket.send(message);
                }

                pending_messages.current = [];
            };

            new_socket.onmessage = async (event) =>
            {
                console.log("Received:", event.data);

                let data;

                try
                {
                    data = JSON.parse(event.data);
                }
                catch
                {
                    on_message_ref.current(event.data);

                    return;
                }

                if (data.type === "key_exchange")
                {
                    if (!private_key.current)
                    {
                        return;
                    }

                    try
                    {
                        shared_key.current =
                            await derive_shared_key(
                                private_key.current,
                                data.public_key
                            );

                        console.log(
                            "Shared encryption key established."
                        );
                    }
                    catch (error)
                    {
                        console.error(
                            "Failed to establish shared key:",
                            error
                        );
                    }

                    return;
                }

                if (data.type === "encrypted")
                {
                    if (!shared_key.current)
                    {
                        console.error(
                            "Received encrypted message before shared key."
                        );

                        return;
                    }

                    try
                    {
                        const decrypted =
                            await decrypt_message(
                                {
                                    iv: data.iv,
                                    ciphertext: data.ciphertext
                                },
                                shared_key.current
                            );

                        console.log(
                            "Decrypted:",
                            decrypted
                        );

                        on_message_ref.current(
                            decrypted
                        );
                    }
                    catch (error)
                    {
                        console.error(
                            "Failed to decrypt message:",
                            error
                        );
                    }

                    return;
                }

                on_message_ref.current(event.data);
            };

            new_socket.onerror = (error) =>
            {
                console.error(
                    "WebSocket error:",
                    error
                );
            };

            new_socket.onclose = () =>
            {
                console.log("WebSocket closed");

                if (socket.current === new_socket)
                {
                    socket.current = null;
                }
            };
        }

        connect();

        return () =>
        {
            cancelled = true;

            if (socket.current)
            {
                socket.current.close();
                socket.current = null;
            }

            private_key.current = null;
            shared_key.current = null;
        };
    }, []);

    async function send_message
    (
        message: string,
        encrypted: boolean = true
    )
    {
        if (
            socket.current &&
            socket.current.readyState === WebSocket.OPEN
        )
        {
            if (encrypted && shared_key.current)
            {
                const encrypted_message =
                    await encrypt_message(
                        message,
                        shared_key.current
                    );

                socket.current.send(
                    JSON.stringify({
                        type: "encrypted",
                        iv: encrypted_message.iv,
                        ciphertext: encrypted_message.ciphertext
                    })
                );
            }
            else
            {
                socket.current.send(message);
            }

            return;
        }

        console.log(
            "WebSocket not ready, queued:",
            message
        );

        pending_messages.current.push(message);
    }

    return {
        send_message
    };
}

export default useChatSocket;