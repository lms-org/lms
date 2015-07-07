##Contributing to LMS:

//TODO link getting started working with source

### API Changes & Additions
If you modify a public API, or add a new one, make sure to add these changes to the [CHANGES](https://github.com/Phibedy/LMS/CHANGES) file in the root of the repository.
If you want to make sure, that your changes will be included start a conversation via github issues or pullrequests.

### Contributor License Agreement

LMS is licensed under the [Apache 2.0 license](http://www.apache.org/licenses/LICENSE-2.0.html).
//TODO Contributor License Agreement (CLA)

### Code Style

We have some code-guidelines //TODO add link, in short:

A few things we'd like to see:

  * It's a C++ project, use C++ and avoid C
  * Use & for * see //TODO insert link
  * code => CamelCase
  * files => snake_case

If you create a new file:
  * make sure to add the Apache file header, as seen //TODO add link
  * if the name isn't self explained please add a README that states what the file is for
If you create a new source-file, please add documentation that explains the usage and scope of he src and add comment your methods and difficult parts in your code.
No comments = no code

### Performance Considerations

LMS is meant to run fast and reliable.

A couple of guidelines:
  * Avoid temporary object allocation
  * Do not perform argument checks for methods (in some cases they might be usefull). The person who calls your method is bound to check them
  * Use pooling if necessary, if possible, avoid exposing the pooling to the user as it complicates the API

### Git

Pull requests for the master repository will be checked before inclusion.
