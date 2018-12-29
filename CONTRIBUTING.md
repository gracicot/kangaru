Contributing
============

Kangaru has benefitted a lot from patches and would benefit a lot from your help. Every commit counts!

To submit a patch, fork the repo on your account and clone it:
```sh
$ git clone git@github.com:your-username/kangaru.git && cd kangaru
```

Then, setup the project using CMake. As opposed to the default setup, we will enable tests and examples:
```sh
mkdir build && cd build
cmake .. -DKANGARU_BUILD_EXAMPLES=On -DKANGARU_TEST=On -DKANGARU_TEST_CXX14=On -DKANGARU_TEST_CXX17=On
```

You can build and test your project using your favourite IDE that supports CMake, or build using the command line:
```sh
cmake --build .
ctest
```

You're then ready to make your changes! To ensure your changes work as expected, simply build and test again.

Finally, push your patch to your fork and [submit a pull request](https://github.com/gracicot/kangaru/compare/).

To finish the process, we will review and comment your pull request. We will respond as fast as possible, but it might take a day or two.

To increase the chances of a patch to be accepted, please be mindful of:

 * Writing a descriptive but consice commit message,
 * Follow the coding style of the edited file,
 * Write tests if applicable.
 
