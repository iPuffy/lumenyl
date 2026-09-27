import { useEffect, useRef } from "react";

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
    const pending_messages = useRef<string[]>([]);
    const on_message_ref = useRef(on_message);

    useEffect(() =>
    {
        on_message_ref.current = on_message;
    }, [on_message]);

    useEffect(() =>
    {
        const protocol =
            window.location.protocol === "https:"
                ? "wss:"
                : "ws:";

        const socket_url =
            `${protocol}//${window.location.host}/chat`;

        console.log("Connecting to:", socket_url);

        const new_socket = new WebSocket(socket_url);

        socket.current = new_socket;

        new_socket.onopen = () =>
        {
            console.log("WebSocket connected");

            for (const message of pending_messages.current)
            {
                new_socket.send(message);
            }

            pending_messages.current = [];
        };

        new_socket.onmessage = (event) =>
        {
            console.log("Received:", event.data);

            on_message_ref.current(event.data);
        };

        new_socket.onerror = (error) =>
        {
            console.error("WebSocket error:", error);
        };

        new_socket.onclose = () =>
        {
            console.log("WebSocket closed");

            if (socket.current === new_socket)
            {
                socket.current = null;
            }
        };

        return () =>
        {
            new_socket.close();

            if (socket.current === new_socket)
            {
                socket.current = null;
            }
        };
    }, []);

    function send_message(message: string)
    {
        if (
            socket.current &&
            socket.current.readyState === WebSocket.OPEN
        )
        {
            socket.current.send(message);

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