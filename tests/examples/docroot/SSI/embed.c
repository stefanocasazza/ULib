int main(void) {
   struct mg_context *ctx;
  const char *options[] = {"listening_ports", LISTENING_PORT, NULL};

   ctx = mg_start(callback, options);
  pause();
  return 0;
}
