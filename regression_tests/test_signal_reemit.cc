#include <glom/signal_reemitter.h>
#include <iostream>

void on_reemit_void()
{
  std::cout << "Success: signal_to_reemit_void was emitted when signal_first_emit was emitted." << std::endl;
}

void on_reemit_int(int param)
{
  std::cout << "Success: signal_to_reemit_int was emitted when signal_first_emit was emitted. param=" << param << std::endl;
}

int main()
{
  {
    sigc::signal<void> signal_first_emit;
    sigc::signal<void> signal_to_reemit;

    Glom::signal_connect_for_reemit_0args(signal_first_emit, signal_to_reemit);
    signal_to_reemit.connect( sigc::ptr_fun(&on_reemit_void) );

    signal_first_emit.emit();
  }

  {
    sigc::signal<void, int> signal_first_emit;
    sigc::signal<void, int> signal_to_reemit;

    Glom::signal_connect_for_reemit_1arg(signal_first_emit, signal_to_reemit);
    signal_to_reemit.connect( sigc::ptr_fun(&on_reemit_int) );

    signal_first_emit.emit(1);
  }
}
