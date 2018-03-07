#include "nvmveri.hh"
#include "common.hh"

const int LOOP_CNT = 100;

int main()
{
	NVMVeri nvm_instance;

	vector<Metadata> m;
	Metadata dat;

	Timer timer;

	timer.startTimer();
	for (int i = 0; i < LOOP_CNT; i++) {
		printf("%d\n", i);
		m.push_back(dat);
		nvm_instance.execVeri(&m);
	}

	vector<VeriResult> r;
	nvm_instance.getVeri(r);

	timer.endTimer();

	printf("Total time for %d tasks = %llu(us)\n", LOOP_CNT, timer.getTime());
	return 0;
}
