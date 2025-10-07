import { defineConfig } from 'vite';

export default defineConfig({
  root: __dirname,
  server: {
  open: false,
  port: 5173,
  strictPort: true
  }
});
