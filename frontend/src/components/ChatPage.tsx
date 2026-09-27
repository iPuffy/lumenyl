import { useEffect, useRef } from "react";
import type { FormEvent } from "react";

interface ChatMessage
{
    text: string;
    sender: "listener" | "talker" | "system";
}

interface ChatPageProps
{
    role: "listener" | "talker" | "system";

    messages: ChatMessage[];
    on_send_message: (message: string) => void;
    on_leave: () => void;
}

function ChatPage
(
    {
        role,
        messages,
        on_send_message,
        on_leave
    }: ChatPageProps
)
{

    const messages_end_ref = useRef<HTMLDivElement>(null);

    useEffect(() =>
    {
        messages_end_ref.current?.scrollIntoView({
            behavior: "smooth"
        });
    }, [messages]);

    function handle_submit(event: FormEvent<HTMLFormElement>)
    {
        event.preventDefault();

        const input = event.currentTarget.elements.namedItem("message") as HTMLInputElement;

        const message = input.value.trim();

        if (!message)
        {
            return;
        }

        on_send_message(message);

        input.value = "";
    }

    return (
        <main className="chat-page">
            <header className="chat-header">
                <div>
                    <h1>Lumenyl</h1>
                    <span>You are the {role}</span>
                </div>

                <button
                    className="leave-button"
                    onClick={on_leave}
                >
                    Leave
                </button>
            </header>

            <section className="messages">
                {messages.length === 0 && (
                    <div className="empty-chat">
                        <div className="empty-chat-icon">
                        </div>

                        <h2>You’re connected.</h2>

                        <p>
                            Take your time. There's no pressure to start
                            with anything in particular.
                        </p>
                    </div>
                )}

                {messages.map((message, index) => (
                    <div
                        className={
                            message.sender === "system"
                                ? "message system"
                                : message.sender === role
                                    ? "message sent"
                                    : "message received"
                        }
                        key={index}
                    >
                        {message.text}
                    </div>
                ))}
                <div ref={messages_end_ref} />
            </section>

            <form
                className="message-input-container"
                onSubmit={handle_submit}
            >
                <input
                    name="message"
                    type="text"
                    placeholder="Write something..."
                    autoComplete="off"
                />

                <button type="submit">
                    Send
                </button>
            </form>
        </main>
    );
}

export default ChatPage;