


void _generate_checksum() {
    CK_A = 0, CK_B = 0
    For(I=0;I<N;I++)
    {
        CK_A = CK_A + Buffer[I]
        CK_B = CK_B + CK_A
    }
}
