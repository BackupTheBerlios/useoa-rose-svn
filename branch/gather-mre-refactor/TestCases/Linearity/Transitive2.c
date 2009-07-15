! LinearityAnalysis for Automatic Differentiation
!
!Result Should be:
!<<a,x>,nonlinear>
!<<y,a>,linear>, <<y,b>,linear>, <<y,x>,nonlinear>
!<<z,y>,linear>, <<z,x>,nonlinear>, <<z,a>,linear>, <<z,b>,linear>
!      
        program main
          double precision :: a, b, c, x, y, z
          a = 0.0
          b = 0.0
          c = 0.0
          x = 0.0
          y = 0.0
          z = 0.0

          a = x * x
          y = a + b
          z = y + x
        end program main
        
