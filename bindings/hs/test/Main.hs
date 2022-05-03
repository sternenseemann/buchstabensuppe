module Main (main) where

import Test.Buchstabensuppe.Buffers

import Test.Tasty

main :: IO ()
main = defaultMain $ testGroup "Tests"
  [ buffers
  ]
