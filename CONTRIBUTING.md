# Contributing to Fast Feed Parser

We welcome contributions to the Fast Feed Parser project! This document provides guidelines for contributing to ensure a smooth collaboration process.

## Code of Conduct

This project adheres to a code of conduct that we expect all contributors to follow. Please be respectful and professional in all interactions.

## Getting Started

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.15 or higher
- Git for version control
- Basic understanding of lock-free programming concepts

### Development Environment Setup

1. Fork the repository
2. Clone your fork locally:
   ```bash
   git clone https://github.com/your-username/fast-feed-parser.git
   cd fast-feed-parser
   ```
3. Create a development branch:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Guidelines

### Code Style

We follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some modifications:

- Use 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- Use `snake_case` for variables and functions
- Use `PascalCase` for classes and structs
- Use `UPPER_CASE` for constants and macros

### Code Formatting

Use `clang-format` with the provided configuration:

```bash
# Format all source files
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

### Documentation Standards

- All public APIs must have Doxygen documentation
- Include usage examples for complex functions
- Document performance characteristics where relevant
- Update README.md for user-facing changes

### Performance Requirements

This is a high-performance system. All contributions must:

- Maintain or improve existing performance characteristics
- Include benchmarks for performance-critical changes
- Avoid memory allocations in hot paths
- Use appropriate memory ordering for atomic operations

## Testing

### Running Tests

```bash
mkdir build && cd build
cmake -DBUILD_TESTING=ON ..
make test
```

### Test Requirements

- All new features must include comprehensive tests
- Maintain 100% test coverage for new code
- Include both unit tests and integration tests
- Performance tests for latency-critical components

### Benchmark Testing

Run performance benchmarks to ensure no regressions:

```bash
# Latency benchmark
./fast-feed-parser 100000 60 16

# Throughput benchmark
./fast-feed-parser 2000000 30 20
```

## Submission Process

### Pull Request Guidelines

1. **Create a descriptive title**: Use format "feat: add new feature" or "fix: resolve issue"
2. **Provide detailed description**: Explain what changes were made and why
3. **Include test results**: Show that tests pass and performance is maintained
4. **Reference issues**: Link to related GitHub issues if applicable

### Pull Request Template

```markdown
## Description
Brief description of changes made.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Performance improvement
- [ ] Documentation update

## Testing
- [ ] Unit tests pass
- [ ] Integration tests pass
- [ ] Performance benchmarks show no regression
- [ ] Manual testing completed

## Performance Impact
Describe any performance implications of your changes.

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Code is commented, particularly in hard-to-understand areas
- [ ] Documentation updated
- [ ] No new compiler warnings introduced
```

### Review Process

1. **Automated checks**: CI/CD pipeline runs tests and checks
2. **Code review**: At least one maintainer reviews the code
3. **Performance review**: Performance-critical changes get additional review
4. **Documentation review**: Ensure documentation is accurate and complete

## Issue Reporting

### Bug Reports

Include the following information:

- **Environment**: OS, compiler version, hardware specs
- **Steps to reproduce**: Detailed steps to reproduce the issue
- **Expected behavior**: What you expected to happen
- **Actual behavior**: What actually happened
- **Performance impact**: If applicable, include benchmark results

### Feature Requests

- **Use case**: Describe the problem you're trying to solve
- **Proposed solution**: Your suggested approach
- **Alternatives considered**: Other approaches you've considered
- **Performance requirements**: Any specific performance needs

## Architecture Guidelines

### Lock-Free Programming

When working with lock-free code:

- Understand memory ordering semantics
- Use appropriate atomic operations
- Test thoroughly on multiple architectures
- Document memory ordering choices
- Consider ABA problems and mitigation strategies

### Performance Considerations

- Profile before optimizing
- Measure the impact of changes
- Consider cache line effects
- Minimize memory allocations
- Use appropriate data structures for access patterns

## Release Process

### Version Numbering

We use [Semantic Versioning](https://semver.org/):

- **MAJOR**: Incompatible API changes
- **MINOR**: Backward-compatible functionality additions
- **PATCH**: Backward-compatible bug fixes

### Release Checklist

- [ ] All tests pass
- [ ] Performance benchmarks meet requirements
- [ ] Documentation updated
- [ ] CHANGELOG.md updated
- [ ] Version numbers updated
- [ ] Release notes prepared

## Getting Help

### Communication Channels

- **GitHub Issues**: For bug reports and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Email**: [imtiaz.qureshi@gmail.com](mailto:imtiaz.qureshi@gmail.com) for private matters

### Resources

- [Project Documentation](README.md)
- [API Documentation](docs/api.md)
- [Performance Guide](docs/performance.md)
- [Architecture Overview](docs/architecture.md)

## Recognition

Contributors will be recognized in:

- CONTRIBUTORS.md file
- Release notes for significant contributions
- Annual contributor acknowledgments

Thank you for contributing to Fast Feed Parser!