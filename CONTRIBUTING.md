## Contributing to LMS:

//TODO link getting started working with source

### API Changes & Additions
If you modify a public API, or add a new one, make sure to add these changes to the [CHANGES](https://github.com/Phibedy/LMS/CHANGES) file in the root of the repository.
If you want to make sure, that your changes will be included start a conversation via github issues or pullrequests.

### Contributor License Agreement

LMS is licensed under the [Apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0.html).

//TODO Contributor License Agreement (CLA)

### Code Style

We have some code-guidelines:
- [Google Styleguide](google-styleguide.googlecode.com/svn/trunk/cppguide.html)
- [C++ Coding Standard](www.possibility.com/Cpp/CppCodingStandard.html)

We do not enforce the style guides but you can learn a lot by just reading.

A few things we'd like to see:

- It's a C++ project, use C++ and avoid C
- Use references (&) instead of pointers (\*), whenever possible
- C++ Classes => UpperCamelCase
- C++ Class methods and attributes => lowerCamelCase
- C/C++ constants and enum values => TITLE_CASE
- file names => snake_case

If you create a new file:
- make sure to add the Apache file header, as seen //TODO add link
- if the name isn't self explained please add a README that states what the file
  is for
- If you create a new source-file, please add documentation that explains the
  usage and scope of he src and add comment your methods and difficult parts in
  your code.
- No comments = no code

### Every module and library
- Include `README.md` with information of the author, the creation date
  and its usage

### Every .CPP, .H file
- Comment with information of the author, the creation date and its
  usage / *What is it doing?*
- For every method: Comment the method if the name is not self-descriptive
- For every coding hack include a `TODO HACK` comment with an explanation
- Use header guards in every header file
- Write a *TODO* comment at the file's top if incomplete

### Performance Considerations

LMS is meant to run fast and reliable.

A couple of guidelines:
- Avoid temporary object allocation
- Do not perform argument checks for methods (in some cases they might be
  useful). The person who calls your method is bound to check them
- Use pooling if necessary, if possible, avoid exposing the pooling to the user
  as it complicates the API

### Git

- Pull requests for the master repository will be checked before inclusion.
- Push only compilable, working code on the *master* branch.
- Create branches for features and issues, not for persons.
- Merge feature branches with master if they work and are well tested.
- Create branches of branches if you implement a sub feature.
