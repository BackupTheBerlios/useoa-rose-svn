! LinearityAnalysis for Automatic Differentiation
!
!Result Should be:
!      
        program main
          double precision :: a, b, c, x, y, z
          a = 0.0
          b = 0.0
          c = 0.0
          x = 0.0
          y = 0.0
          z = 0.0

          y = x
          a = a + y
          y = y * x
          b = a
        end program main
        
