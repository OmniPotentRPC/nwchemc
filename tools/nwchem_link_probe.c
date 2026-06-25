extern int task_gradient_(const int *rtdb);

void nwchemc_link_probe(void) {
  const int rtdb = 0;
  (void)task_gradient_(&rtdb);
}
