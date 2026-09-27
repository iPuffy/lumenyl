interface StartPageProps
{
    on_choose_role: (role: "listener" | "talker") => void;
}

function StartPage({ on_choose_role }: StartPageProps)
{
    return (
        <main className="landing-page">
            <section className="hero">
                <h1>Lumenyl</h1>

                <p className="subtitle">
                    Anonymously talk about anything with anyone
                </p>

                <div className="choice-container">
                    <button
                        className="choice-card"
                        onClick={() => on_choose_role("listener")}
                    >
                        <span className="choice-title">
                            I want to listen
                        </span>

                        <span className="choice-description">
                            Be there for someone who needs to talk
                        </span>
                    </button>

                    <button
                        className="choice-card"
                        onClick={() => on_choose_role("talker")}
                    >
                        <span className="choice-title">
                            I want to talk
                        </span>

                        <span className="choice-description">
                            Find someone willing to listen
                        </span>
                    </button>
                </div>
            </section>
        </main>
    );
}

export default StartPage;