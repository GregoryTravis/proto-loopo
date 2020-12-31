-- This file is part of Hoppy.
--
-- Copyright 2015-2020 Bryan Gardiner <bog@khumba.net>
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

module Main (main) where

import Foreign.Hoppy.Setup (ProjectConfig (..), hsMain)
import qualified Foreign.Hoppy.Example.Generator as Generator

main =
  hsMain
  ProjectConfig
  { interfaceResult = Generator.interfaceResult
  , cppPackageName = "hoppy-example-cpp"
  , cppSourcesDir = "cpp"
  , hsSourcesDir = "Source"
  }
