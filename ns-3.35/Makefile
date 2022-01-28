configure:
	./waf -j8 configure --enable-examples --enable-tests

compile:
	./waf -j8
run:
	./waf -j8 
	./waf -j8 --run scratch/$(t)

trace: run
	java -jar ./../tracemetrics/tracemetrics.jar scratch/$(t)

netanim: run
	./../netanim-3.108/NetAnim
second:
	./waf -j8
	./waf -j8 --run "scratch/second --nCsma=5 --verbose=true" NS_LOG="SecondScriptExample"

third: compile
	./waf -j8 --run "scratch/third --nCsma=5 nWifi=2 --verbose=true --tracing=true" NS_LOG="ThirdScriptExample"