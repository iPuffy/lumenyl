import { useCallback, useState } from "react";

import "./App.css";

import StartPage from "./components/StartPage";
import WaitingPage from "./components/WaitingPage";
import ChatPage from "./components/ChatPage";

import useChatSocket from "./hooks/UseChatSocket";

interface ChatMessage
{
    text: string;
    sender: "listener" | "talker" | "system";
}

function App()
{
    const [page, set_page] =
        useState<"start" | "waiting" | "chat">("start");

    const [role, set_role] =
        useState<"listener" | "talker" | null>(null);

    const [messages, set_messages] =
        useState<ChatMessage[]>([]);

    const handle_message = useCallback((message: string) =>
    {
        const data = JSON.parse(message);

        if (data.type === "system")
        {
            if (data.event === "matched")
            {
                set_page("chat");
            }

            set_messages(current => [
                ...current,
                {
                    text: data.message,
                    sender: "system"
                }
            ]);

            return;
        }

        if (data.type === "chat")
        {
            set_messages(current => [
                ...current,
                {
                    text: data.message,
                    sender: data.sender
                }
            ]);

            return;
        }
    }, []);

    const { send_message } = useChatSocket
    (
        {
            on_message: handle_message
        }
    );

    function choose_role(new_role: "listener" | "talker")
    {
        set_role(new_role);
        set_messages([]);

        send_message(new_role, false);

        set_page("waiting");
    }

    function cancel_matchmaking()
    {
        send_message("cancel", false);

        set_role(null);

        set_page("start");
    }

    function send_chat_message(message: string)
    {
        send_message(
            JSON.stringify({
                type: "chat",
                message
            })
        );

        set_messages(current => [
            ...current,
            {
                text: message,
                sender: role!
            }
        ]);
    }

    function leave_chat()
    {
        send_message("leave", false);

        set_messages([]);
        set_role(null);
        set_page("start");
    }

    if (page === "start")
    {
        return (
            <StartPage
                on_choose_role={choose_role}
            />
        );
    }

    if (page === "waiting")
    {
        return (
            <WaitingPage
                role={role!}
                on_cancel={cancel_matchmaking}
            />
        );
    }

    return (
        <ChatPage
            role={role!}
            messages={messages}
            on_send_message={send_chat_message}
            on_leave={leave_chat}
        />
    );
}

export default App;