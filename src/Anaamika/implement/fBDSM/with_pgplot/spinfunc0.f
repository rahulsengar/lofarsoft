
        subroutine spinfunc0(x,a,y,dyda,na)
        implicit none
        integer na,i
        real*8 x,y,a(na),dyda(na)

        y=a(1)+a(2)*x

        dyda(1)=1.d0
        dyda(2)=x

        return
        end
