! LinearityAnalysis for Automatic Differentiation Simple Example
        program main
          double precision :: a, b, c
          a = 0.0
          b = 0.0
          c = 0.0

          a = b * b
          b = c + a
          c = a
        end program main
        
