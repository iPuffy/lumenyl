interface WaitingPageProps
{
    role: "listener" | "talker";

    on_cancel: () => void;
}

function WaitingPage
    (
        {
            role,
            on_cancel
        }: WaitingPageProps
    )
{
    return (
        <main className="landing-page">
            <section className="hero waiting-card">
                <div className="waiting-loader" />

                <h1>Looking for someone...</h1>

                <p className="subtitle">
                    {
                        role === "listener"
                            ? "We're finding someone who would like to talk."
                            : "We're finding someone who's willing to listen."
                    }
                </p>

                <button
                    className="cancel-button"
                    onClick={on_cancel}
                >
                    Cancel
                </button>
            </section>
        </main>
    );
}

export default WaitingPage;