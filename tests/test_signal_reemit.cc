#include <glom/signal_reemitter.h>
#include <iostream>
#include <cstdlib>

bool success_reemit_void = false;
bool success_reemit_int = false;

const int param_value = 1;

void on_reemit_void()
{
  //std::cout << "Success: signal_to_reemit_void was emitted when signal_first_emit was emitted.\n";
  success_reemit_void = true;
}

void on_reemit_int(int param)
{
  //std::cout << "Success: signal_to_reemit_int was emitted when signal_first_emit was emitted. param=" << param << std::endl;
  success_reemit_int = (param_value == param);
}

int main()
{
  {
    sigc::signal<void()> signal_first_emit;
    sigc::signal<void()> signal_to_reemit;

    Glom::signal_connect_for_reemit_0args(signal_first_emit, signal_to_reemit);
    signal_to_reemit.connect( sigc::ptr_fun(&on_reemit_void) );

    signal_first_emit.emit();
  }

  {
    sigc::signal<void(int)> signal_first_emit;
    sigc::signal<void(int)> signal_to_reemit;

    Glom::signal_connect_for_reemit_1arg(signal_first_emit, signal_to_reemit);
    signal_to_reemit.connect( sigc::ptr_fun(&on_reemit_int) );

    signal_first_emit.emit(param_value);
  }

  if(success_reemit_void && success_reemit_int)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
