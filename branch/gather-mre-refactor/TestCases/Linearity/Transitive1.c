! LinearityAnalysis for Automatic Differentiation Simple Example
        program main
          double precision :: a, b, c, x
          a = 0.0
          b = 0.0
          c = 0.0
          x = 0.0

          a = b * b
          b = c + 1
          c = x + x
        end program main
        
