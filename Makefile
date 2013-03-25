
subdir-y = \
    src \
    test

test_depends-y = \
    src

include Makefile.lib

tests: test
	find test -maxdepth 1 -name "success-*-debug" -exec bash -c 'echo ""; echo "testing {}..."; {}; if [ "$$?" != "0" ]; then echo "fail"; else echo "success"; fi; sleep 1' \;
	find test -maxdepth 1 -name "fail-*-debug" -exec bash -c 'echo ""; echo "testing {}..."; {}; if [ "$$?" == "0" ]; then echo "fail"; else echo "success"; fi; sleep 1' \;
	