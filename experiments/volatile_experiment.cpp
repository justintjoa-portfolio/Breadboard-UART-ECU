#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

namespace {

bool plain_ready = false;
volatile bool volatile_ready = false;
std::atomic<bool> atomic_ready{false};

void SleepBriefly() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void SetPlainReadyLater() {
  SleepBriefly();
  plain_ready = true;
}

void SetVolatileReadyLater() {
  SleepBriefly();
  volatile_ready = true;
}

void SetAtomicReadyLater() {
  SleepBriefly();
  atomic_ready.store(true);
}

void RunPlainBoolExperiment() {
  plain_ready = false;

  std::thread setter(SetPlainReadyLater);

  while (!plain_ready) {
  }

  setter.join();
  std::cout << "plain bool loop exited, but this version has a data race.\n";
}

void RunVolatileBoolExperiment() {
  volatile_ready = false;

  std::thread setter(SetVolatileReadyLater);

  while (!volatile_ready) {
  }

  setter.join();
  std::cout << "volatile bool loop exited, but volatile is still not thread synchronization.\n";
}

void RunAtomicBoolExperiment() {
  atomic_ready.store(false);

  std::thread setter(SetAtomicReadyLater);

  while (!atomic_ready.load()) {
  }

  setter.join();
  std::cout << "atomic bool loop exited correctly.\n";
}

}  // namespace

int main() {
  std::cout << "Running volatile experiment...\n";

  RunPlainBoolExperiment();
  RunVolatileBoolExperiment();
  RunAtomicBoolExperiment();

  return 0;
}