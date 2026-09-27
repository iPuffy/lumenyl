export class WebSocketService
{
    private socket: WebSocket | null = null;

    connect(
        on_open: () => void,
        on_message: (message: string) => void,
        on_close: () => void,
        on_error: () => void
    )
    {
        this.socket = new WebSocket("ws://localhost:8848/chat");

        this.socket.onopen = () =>
        {
            on_open();
        };

        this.socket.onmessage = (event) =>
        {
            on_message(event.data);
        };

        this.socket.onclose = () =>
        {
            on_close();
        };

        this.socket.onerror = () =>
        {
            on_error();
        };
    }

    send(message: string)
    {
        if (this.socket && this.socket.readyState === WebSocket.OPEN)
        {
            this.socket.send(message);
        }
    }

    disconnect()
    {
        if (this.socket)
        {
            this.socket.close();
            this.socket = null;
        }
    }
}