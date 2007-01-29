! LinearityAnalysis for Automatic Differentiation
!
!Result Should be:
!<<a,a>,linear>, <<a,y>,linear>, <<a,x>,nonlinear>
!<<y,y>,nonlinear>, <<y,x>,nonlinear>
!      
        program main
          double precision :: a, b, c, x, y, z
          a = 0.0
          b = 0.0
          c = 0.0
          x = 0.0
          y = 0.0
          z = 0.0

          a = 0
          y = x
          y = y * x
          a = a + y
        end program main
        
