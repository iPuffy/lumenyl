import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

export default defineConfig({
    plugins: [react()],

    server: {
        proxy: {
            "/chat": {
                target: "ws://127.0.0.1:10000",
                ws: true
            }
        }
    }
});