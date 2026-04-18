# Contributing

Kangaru has benefitted a lot from patches from the community and would benefit a lot from your help. Every commit counts!

## Expectations

As with any collaborative development environment, it is the expectation that contributors engages with
the maintainers and help them getting contributions up to the standard of code quality and style of this project.

We expect contributors to be familiar with the git workflow and github PR process, and we recommend
having some familiarity with C++ and CMake.

We don't expect everyone to be familiar with this particular codebase, and that's ok! We will be glad to answer questions
and we value sharing knowledge between contributors of this project.

### AI Policy

Contributors are responsible to understand code they submit to PRs. We do not allow LLMs impersonating humans, and all
text generated using an LLM has to be disclosed as such, even if used for translation purpose.

At the moment, we do not allow using LLMs for generating code, test or documentation. Using LLMs to debug or review
contributions is at discretion of the contributor. They can sometimes be helpful to decipher large compiler error
output for example.

## Setup

To submit a patch, fork the repo to your account and clone it:
```sh
git clone git@github.com:your-username/kangaru.git && cd kangaru
```

To have a fully working dev environment, nix can be used so all necessary tools become available for a good
developer experience:
```sh
nix develop
```

If nix is not available on your system, you will need a working supported compiler, cmake, ninja and
preferably vcpkg for package management.

Then, setup the project using CMake. We're using presets to make the development setup easier to get started:
```sh
cmake --preset dev-linux
```
Many other profiles exist, and are all on the form of `<env>-<platform>`

You can build and test your project using your favourite IDE that supports CMake, or build using the command line:
```sh
cmake --build --preset dev-linux-debug
```
Most build profile are using the form `<env>-<platform>-<build-type>`

Once you have a working build, you can run the test using `ctest`:
```sh
ctest --preset dev-linux-debug
```
The test preset always has the same name as the build preset.

## Writing a Patch

You're then ready to make your changes! Don't forget to adjust and / or create tests for the changes.

Finally, push your commit to your fork and [submit a pull request](https://github.com/gracicot/kangaru/compare/).

To finish the process, we will review and comment your pull request. We will respond as fast as possible, but depending
on our availability it can take time. Be mindful that most contributors are maintaining this project in their own free time.

To increase the chances of a patch to be accepted consider the following:

 * Writing a descriptive but consice commit message,
 * Follow the coding style of the edited files,
 * Write tests that ensures the changes are correct,
 * Include documentation adjustments alongside the changes.
