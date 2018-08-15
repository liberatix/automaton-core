# Contribution Guidelines

## Style Guides

### C++ Style Guide

We follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some exceptions:

* Names for classes and functions are all lowercase, with underscores between words instead of Camel Case.

## Commits

Few simple rules when making commits:

* Short descriptive sentence for the title (50 characters limit): start with capital letter, do not use "." to end the sentence
* Add description and focus on the why and what, not the how
* Always use a branch, and submit pull requests, which should go through a code review before committed to the master branch
* Do not create too many branches and remove branches you are no longer working on

https://gist.github.com/hofmannsven/6814451

### Always use squash and merge

When you do squash and merge, your repo will hang onto the commits that have already been squashed and merged into the master.

**WARNING! THIS WILL WIPE YOUR LOCAL CHANGES!**

<SPAN style="color: #F00">**ONLY DO THIS AFTER YOU'VE SQUASHED AND MERGED ALL YOUR CHANGES INTO THE MASTER***</SPAN>

```
git reset --hard origin/master
```

## Lint

Check your code using the following lint command:

```
find . -iname *.cc -o -iname *.cpp -o -iname *.h | xargs cpplint.py --filter=-legal/copyright,-build/header_guard
```
