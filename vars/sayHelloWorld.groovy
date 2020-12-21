#!/usr/bin/env groovy

def call(String name = 'human') {
  echo "Hello, I am getting called from local shared library !!!!!! ${name}."
}
