C     nwchem_embed_legacy.F - thin NWChem F77/rtdb/ga/geom/bas calls only.
C     Called from nwchem_embed_c_api.f90 (bind(C)). Fixed-form required for
C     NWChem #include headers. MA sizes come from NWChem Memory_Defaults
C     (env / .nwchemrc), not embed-local constants.

      subroutine nwchem_legacy_init(rtdb, ok, owns_mpi)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "cfileprefix.fh"
      integer rtdb, ok, owns_mpi
      logical status
      character*256 rtdb_name
      integer stack, heap, global, total
      character*1 tsrc, hsrc, ssrc, gsrc
      integer g_init
      external memory_defaults, util_file_name
      external util_set_default_scratch_dir
      external util_set_default_permanent_dir
      external util_file_prefix_set, util_file_info_rtdb
      external util_blas_set_num_threads, rtdb_init, nxtask_init
      save g_init
      data g_init /0/
      ok = -1
      rtdb = -1
      owns_mpi = 0
      if (g_init .eq. 0) then
C        Host may already have MPI_Init (C driver / mpirun). Only start MPI
C        from Fortran when embed owns the process (matches in-process embed).
         call nwchemc_mpi_start_if_needed(owns_mpi)
C        Same path as nwchem.F when no input "memory" directive: defaults
C        from environment / .nwchemrc / compiled-in limits.
         tsrc = 'N'
         hsrc = 'N'
         ssrc = 'N'
         gsrc = 'N'
         call memory_defaults(total, tsrc, heap, hsrc, stack,
     $        ssrc, global, gsrc)
C        Match nwchem.F order: GA limited init, then MA (GA may use MA).
C        Fallback if memory_defaults yields zero (no .nwchemrc / env).
         if (stack .le. 0) stack = 4000000
         if (heap .le. 0) heap = 4000000
         if (global .le. 0) global = 8000000
C        Match NWChemWrap / geninterface embed path (not full nwchem.F).
         call ga_initialize()
         if (ga_uses_ma()) then
            if (.not. ma_init(mt_dbl, stack, heap + global)) return
         else
            if (.not. ma_init(mt_dbl, stack, heap)) return
            call ga_set_memory_limit(ma_sizeof(mt_dbl, global, mt_byte))
         endif
         call util_blas_set_num_threads(1)
C        NWChem file prefix / scratch / permanent (env: NWCHEM_SCRATCH_DIR,
C        NWCHEM_PERMANENT_DIR, or .nwchemrc; else CWD). No embed-local paths.
         call util_file_prefix_set('nwchemc')
         call util_set_default_scratch_dir(scratch_dir)
         call util_set_default_permanent_dir(permanent_dir)
         g_init = 1
      endif
      if (g_init .lt. 0) return
C     RTDB in permanent/scratch via util_file_name (needs dirs above).
      call util_file_name('nwchemc_embed', .true., .false., rtdb_name)
      status = rtdb_open(rtdb_name, 'empty', rtdb)
      if (.not. status) then
         rtdb = -1
         return
      endif
      call util_file_info_rtdb(rtdb)
      call util_set_rtdb_state(rtdb, .true., .false.,
     $     .false.)
      call geom_hnd_parallel(.true.)
      call nxtask_init(rtdb)
      ok = 0
      return
      end

      subroutine nwchem_legacy_finalize(rtdb, owns_mpi, ok)
      implicit none
#include "global.fh"
#include "rtdb.fh"
      integer rtdb, owns_mpi, ok
      integer*4 ierr4, flag4
      logical status
      ok = 0
      if (rtdb .ge. 0) then
         status = rtdb_close(rtdb, 'delete')
         if (.not. status) ok = -1
         rtdb = -1
      endif
      if (ga_initialized()) then
         call ga_sync()
         call ga_terminate()
      endif
      if (owns_mpi .ne. 0) then
         flag4 = 0
         ierr4 = 0
         call mpi_finalized(flag4, ierr4)
         if (flag4 .eq. 0) call mpi_finalize(ierr4)
      endif
      return
      end

C     Direct embed: numeric geometry and library basis are stored through
C     NWChem RTDB-facing APIs; user stanzas still use NWChem input handlers.
      subroutine nwchem_legacy_energy_grad(rtdb, n_atoms, pos_ang,
     $     atmnrs, basis_name, theory_name, scf_type, input_blocks,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, psp_count, psp_elements, psp_types,
     $     psp_names, energy_h, grad_h_bohr, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "inp.fh"
      integer rtdb, n_atoms, charge, mult, ok
      integer psp_count
      integer dft_direct, dft_smear_on, dft_smear_spinset
      double precision pos_ang(*)
      integer atmnrs(*)
      integer psp_types(*)
      character*64 basis_name
      character*64 theory_name
      character*64 scf_type
      character*4096 input_blocks
      character*16 psp_elements(*)
      character*256 psp_names(*)
      double precision dft_smear_sigma
      double precision energy_h
      double precision grad_h_bohr(*)
      character*(*) errmsg
      integer n3, i
      integer l_grad, k_grad
      integer maxat
      parameter (maxat = 64)
      logical status
      external task_energy, task_gradient
      logical task_energy, task_gradient
      external nw_inp_from_character
      logical nw_inp_from_character

      ok = -1
      energy_h = 0.0d0
      errmsg = ' '
      if (rtdb .lt. 0 .or. n_atoms .le. 0) then
         errmsg = 'invalid rtdb or n_atoms'
         return
      endif
      if (n_atoms .gt. maxat) then
         errmsg = 'n_atoms exceeds embed maxat=64'
         return
      endif

      call nwchemc_store_geometry(rtdb, n_atoms, pos_ang, atmnrs,
     $     errmsg, status)
      if (.not. status) return
      call nwchemc_store_library_basis(rtdb, n_atoms, atmnrs,
     $     basis_name, errmsg, status)
      if (.not. status) return
      call nwchemc_store_pseudopotentials(rtdb, psp_count, psp_elements,
     $     psp_types, psp_names, errmsg, status)
      if (.not. status) return
      if (inp_strlen(input_blocks) .gt. 0) then
         if (.not. nw_inp_from_character(rtdb,
     $        input_blocks(1:inp_strlen(input_blocks)))) then
            errmsg = 'nw_inp_from_character inputBlocks failed'
            return
         endif
      endif

      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, status)
      if (.not. status) then
         errmsg = 'method RTDB setup failed'
         return
      endif
      status = rtdb_cput(rtdb, 'task:operation', 1, 'energy')
      status = rtdb_cput(rtdb, 'task:theory', 1, theory_name)

      if (.not. task_energy(rtdb)) then
         errmsg = 'task_energy/scf failed'
         return
      endif
      if (.not. rtdb_get(rtdb, 'task:energy', mt_dbl, 1, energy_h)) then
         if (.not. rtdb_get(rtdb, 'scf:energy', mt_dbl, 1,
     $        energy_h)) then
            errmsg = 'rtdb_get task/scf energy failed'
            return
         endif
      endif

      n3 = 3 * n_atoms
      status = rtdb_cput(rtdb, 'task:operation', 1, 'gradient')
      if (.not. task_gradient(rtdb)) then
         errmsg = 'task_gradient failed'
         return
      endif
      if (.not. ma_push_get(mt_dbl, n3, 'nwchemc_g', l_grad,
     $     k_grad)) then
         errmsg = 'ma_push_get grad failed'
         return
      endif
      if (.not. rtdb_get(rtdb, 'task:gradient', mt_dbl, n3,
     $        dbl_mb(k_grad))) then
         do i = 1, n3
            grad_h_bohr(i) = 0.0d0
         enddo
         errmsg = 'rtdb_get task:gradient failed; grad zeroed'
      else
         do i = 1, n3
            grad_h_bohr(i) = dbl_mb(k_grad + i - 1)
         enddo
         errmsg = 'ok'
      endif
      status = ma_pop_stack(l_grad)
      ok = 0
      return
      end

      subroutine nwchem_legacy_hessian(rtdb, n_atoms, pos_ang,
     $     atmnrs, basis_name, theory_name, scf_type, input_blocks,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, psp_count, psp_elements, psp_types,
     $     psp_names, hessian_h_bohr2, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "inp.fh"
      integer rtdb, n_atoms, charge, mult, ok
      integer psp_count
      integer dft_direct, dft_smear_on, dft_smear_spinset
      double precision pos_ang(*)
      integer atmnrs(*)
      integer psp_types(*)
      character*64 basis_name
      character*64 theory_name
      character*64 scf_type
      character*4096 input_blocks
      character*16 psp_elements(*)
      character*256 psp_names(*)
      double precision dft_smear_sigma
      double precision hessian_h_bohr2(*)
      character*(*) errmsg
      integer n3, i, j
      integer maxat, idxij, idxji
      parameter (maxat = 64)
      logical status
      double precision hval
      character*255 filehess
      external task_hessian, util_file_name
      logical task_hessian
      external nw_inp_from_character
      logical nw_inp_from_character

      ok = -1
      errmsg = ' '
      if (rtdb .lt. 0 .or. n_atoms .le. 0) then
         errmsg = 'invalid rtdb or n_atoms'
         return
      endif
      if (n_atoms .gt. maxat) then
         errmsg = 'n_atoms exceeds embed maxat=64'
         return
      endif

      n3 = 3 * n_atoms
      do i = 1, n3 * n3
         hessian_h_bohr2(i) = 0.0d0
      enddo

      call nwchemc_store_geometry(rtdb, n_atoms, pos_ang, atmnrs,
     $     errmsg, status)
      if (.not. status) return
      call nwchemc_store_library_basis(rtdb, n_atoms, atmnrs,
     $     basis_name, errmsg, status)
      if (.not. status) return
      call nwchemc_store_pseudopotentials(rtdb, psp_count, psp_elements,
     $     psp_types, psp_names, errmsg, status)
      if (.not. status) return
      if (inp_strlen(input_blocks) .gt. 0) then
         if (.not. nw_inp_from_character(rtdb,
     $        input_blocks(1:inp_strlen(input_blocks)))) then
            errmsg = 'nw_inp_from_character inputBlocks failed'
            return
         endif
      endif

      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, status)
      if (.not. status) then
         errmsg = 'method RTDB setup failed'
         return
      endif
      status = rtdb_cput(rtdb, 'task:operation', 1, 'hessian')
      status = rtdb_cput(rtdb, 'task:theory', 1, theory_name)

      if (.not. task_hessian(rtdb)) then
         errmsg = 'task_hessian failed'
         return
      endif
      if (.not. rtdb_cget(rtdb, 'task:hessian file name', 1,
     $     filehess)) then
         call util_file_name('hess', .false., .false., filehess)
      endif

      open(unit=69, file=filehess, form='formatted', status='old',
     $     access='sequential', err=900)
      do i = 1, n3
         do j = 1, i
            read(69, *, err=910, end=920) hval
            idxij = (i - 1) * n3 + j
            idxji = (j - 1) * n3 + i
            hessian_h_bohr2(idxij) = hval
            hessian_h_bohr2(idxji) = hval
         enddo
      enddo
      close(unit=69, status='keep')
      errmsg = 'ok'
      ok = 0
      return

 900  continue
      errmsg = 'error opening hessian file'
      return
 910  continue
      close(unit=69, status='keep')
      errmsg = 'error reading hessian file'
      return
 920  continue
      close(unit=69, status='keep')
      errmsg = 'unexpected EOF reading hessian file'
      return
      end

      subroutine nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, charge, mult
      integer dft_direct, dft_smear_on, dft_smear_spinset
      character*64 theory_name
      character*64 scf_type
      double precision dft_smear_sigma
      logical ok
      logical status, lval
      integer nopen, ival
      double precision chg
      character*16 vectors_in

      ok = .true.
      chg = dble(charge)
      call nwchemc_rtdb_put_dbl(rtdb, 'charge', chg, status)
      ok = ok .and. status
      call nwchemc_rtdb_put_int(rtdb, 'spin mult', mult, status)
      ok = ok .and. status
      vectors_in = 'atomic'
      if (theory_name(1:3) .eq. 'dft') then
         status = rtdb_cput(rtdb, 'dft:xc', 1, scf_type)
         ok = ok .and. status
         call nwchemc_rtdb_put_int(rtdb, 'dft:mult', mult, status)
         ok = ok .and. status
         nopen = mult - 1
         call nwchemc_rtdb_put_int(rtdb, 'dft:nopen', nopen, status)
         ok = ok .and. status
         if (dft_direct .ne. 0) then
            lval = .true.
            status = rtdb_put(rtdb, 'dft:direct', mt_log, 1, lval)
            ok = ok .and. status
            ival = -1
            call nwchemc_rtdb_put_int(rtdb, 'int2e:filesize',
     $         ival, status)
            ok = ok .and. status
            call nwchemc_rtdb_put_int(rtdb, 'int2e:memsize',
     $         ival, status)
            ok = ok .and. status
         endif
         if (dft_smear_on .ne. 0 .and. dft_smear_sigma .gt. 0.0d0)
     $      then
            call nwchemc_rtdb_put_dbl(rtdb, 'dft:smear_sigma',
     $         dft_smear_sigma, status)
            ok = ok .and. status
            lval = dft_smear_spinset .ne. 0
            status = rtdb_put(rtdb, 'dft:spinset', mt_log, 1, lval)
            ok = ok .and. status
         endif
      else
         status = rtdb_cput(rtdb, 'scf:input vectors', 1, vectors_in)
         ok = ok .and. status
         status = rtdb_cput(rtdb, 'scf:type', 1, scf_type)
         ok = ok .and. status
      endif
      return
      end

      subroutine nwchemc_store_pseudopotentials(rtdb, psp_count,
     $     psp_elements, psp_types, psp_names, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "btdb.fh"
#include "inp.fh"
      integer rtdb, psp_count
      character*16 psp_elements(*)
      integer psp_types(*)
      character*256 psp_names(*)
      character*(*) errmsg
      logical ok
      integer i, l_elem, l_name, psp_type, stored_type
      integer maxpsp
      parameter (maxpsp = 64)
      character*255 rtdb_name
      logical status, lval

      ok = .false.
      if (psp_count .lt. 0 .or. psp_count .gt. maxpsp) then
         errmsg = 'invalid pseudopotential count'
         return
      endif
      do i = 1, psp_count
         l_elem = inp_strlen(psp_elements(i))
         l_name = inp_strlen(psp_names(i))
         if (l_elem .le. 0 .or. l_name .le. 0) then
            errmsg = 'empty pseudopotential entry'
            return
         endif
         psp_type = psp_types(i)
         if (psp_type .eq. 2) then
            rtdb_name = psp_elements(i)(1:l_elem)//':paw_library'
            if (.not. btdb_cput(rtdb, rtdb_name, 1,
     $           psp_names(i)(1:l_name))) then
               errmsg = 'pseudopotential paw_library write failed'
               return
            endif
         else
            rtdb_name = psp_elements(i)(1:l_elem)//':pspw_library'
            if (.not. btdb_cput(rtdb, rtdb_name, 1,
     $           psp_names(i)(1:l_name))) then
               errmsg = 'pseudopotential pspw_library write failed'
               return
            endif
            if (psp_type .eq. 0 .or. psp_type .eq. 1) then
               stored_type = 0
            else if (psp_type .eq. 3) then
               stored_type = 2
            else if (psp_type .eq. 4) then
               stored_type = 3
            else
               errmsg = 'unknown pseudopotential library type'
               return
            endif
            rtdb_name = psp_elements(i)(1:l_elem)//':pspw_library_type'
            call nwchemc_rtdb_put_int(rtdb, rtdb_name, stored_type,
     $         status)
            if (.not. status) then
               errmsg = 'pseudopotential library type write failed'
               return
            endif
         endif
         lval = .true.
         rtdb_name = psp_elements(i)(1:l_elem)//':psp_reset'
         if (.not. rtdb_put(rtdb, rtdb_name, mt_log, 1, lval)) then
            errmsg = 'pseudopotential reset write failed'
            return
         endif
      enddo
      ok = .true.
      return
      end

      subroutine nwchemc_store_geometry(rtdb, n_atoms, pos_ang,
     $     atmnrs, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
#include "stdio.fh"
      integer rtdb, n_atoms
      double precision pos_ang(*)
      integer atmnrs(*)
      character*(*) errmsg
      logical ok
      integer iat, geom, maxat
      parameter (maxat = 64)
      double precision coords(3,maxat)
      double precision charge(maxat)
      double precision mass(maxat)
      double precision scale
      character*16 tags(maxat)
      logical status
      external geom_create, geom_destroy, geom_set_user_units
      external geom_get_user_scale, geom_cart_set, geom_masses_set
      external geom_rtdb_store, geom_atn_to_default_mass
      logical geom_create, geom_destroy, geom_set_user_units
      logical geom_get_user_scale, geom_cart_set, geom_masses_set
      logical geom_rtdb_store, geom_atn_to_default_mass

      ok = .false.
      if (n_atoms .le. 0 .or. n_atoms .gt. maxat) then
         errmsg = 'invalid geometry atom count'
         return
      endif
      if (.not. geom_create(geom, 'geometry')) then
         errmsg = 'geom_create failed'
         return
      endif
      if (.not. geom_set_user_units(geom, 'angstroms')) then
         errmsg = 'geom_set_user_units failed'
         status = geom_destroy(geom)
         return
      endif
      if (.not. geom_get_user_scale(geom, scale)) then
         errmsg = 'geom_get_user_scale failed'
         status = geom_destroy(geom)
         return
      endif
      do iat = 1, n_atoms
         call nwchemc_z_to_elem(atmnrs(iat), tags(iat))
         coords(1,iat) = pos_ang(3 * (iat - 1) + 1) * scale
         coords(2,iat) = pos_ang(3 * (iat - 1) + 2) * scale
         coords(3,iat) = pos_ang(3 * (iat - 1) + 3) * scale
         charge(iat) = dble(atmnrs(iat))
         if (.not. geom_atn_to_default_mass(atmnrs(iat), mass(iat)))
     $      mass(iat) = 0.0d0
      enddo
      if (.not. geom_cart_set(geom, n_atoms, tags, coords, charge)) then
         errmsg = 'geom_cart_set failed'
         status = geom_destroy(geom)
         return
      endif
      if (.not. geom_masses_set(geom, n_atoms, mass)) then
         errmsg = 'geom_masses_set failed'
         status = geom_destroy(geom)
         return
      endif
      call geom_compute_values(geom)
      if (.not. geom_rtdb_store(rtdb, geom, 'geometry')) then
         errmsg = 'geom_rtdb_store failed'
         status = geom_destroy(geom)
         return
      endif
      if (.not. geom_destroy(geom)) then
         errmsg = 'geom_destroy failed'
         return
      endif
      ok = .true.
      return
      end

      subroutine nwchemc_store_library_basis(rtdb, n_atoms, atmnrs,
     $     basis_name, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "baslibraryP.fh"
#include "inp.fh"
      integer rtdb, n_atoms
      integer atmnrs(*)
      character*(*) basis_name
      character*(*) errmsg
      logical ok
      integer iat, j, nlib, ntags, basis, maxat
      parameter (maxat = 64)
      character*16 tags(maxat)
      character*16 tag
      character*255 mydir
      character*255 standard
      character*256 filename
      logical seen
      logical status
      logical osegment, oshell_is_rel, read_sphere
      external bas_create, bas_tag_lib, bas_rtdb_store
      external bas_do_destroy, bas_set_library_name
      external bas_add_ucnt_init, bas_add_ucnt_tidy
      external bas_set_spherical, bas_set_upcase
      logical bas_create, bas_rtdb_store, bas_do_destroy
      logical bas_add_ucnt_init, bas_add_ucnt_tidy
      logical bas_set_spherical, bas_set_upcase

      ok = .false.
      if (n_atoms .le. 0 .or. n_atoms .gt. maxat) then
         errmsg = 'invalid basis atom count'
         return
      endif
      mydir = ' '
      call bas_set_library_name(mydir)
      if (.not. bas_create(basis, 'ao basis')) then
         errmsg = 'bas_create failed'
         return
      endif
      if (.not. bas_add_ucnt_init(basis)) then
         errmsg = 'bas_add_ucnt_init failed'
         status = bas_do_destroy(basis)
         return
      endif
      if (.not. bas_set_spherical(basis, .false.)) then
         errmsg = 'bas_set_spherical failed'
         status = bas_do_destroy(basis)
         return
      endif
      if (.not. bas_set_upcase(basis, .false.)) then
         errmsg = 'bas_set_upcase failed'
         status = bas_do_destroy(basis)
         return
      endif
      ntags = 0
      osegment = .true.
      oshell_is_rel = .false.
      read_sphere = .false.
      nlib = inp_strlen(user_library_name)
      if (nlib .ge. 15) then
         if (user_library_name(nlib - 14:nlib) .eq.
     $      '/libraries.bse/') read_sphere = .true.
      endif
      do iat = 1, n_atoms
         call nwchemc_z_to_elem(atmnrs(iat), tag)
         seen = .false.
         do j = 1, ntags
            if (tags(j) .eq. tag) seen = .true.
         enddo
         if (.not. seen) then
            ntags = ntags + 1
            tags(ntags) = tag
            standard = basis_name
            filename = user_library_name
            call bas_tag_lib(basis, osegment, tag, tag, standard,
     $           filename, oshell_is_rel, read_sphere)
         endif
      enddo
      if (.not. bas_rtdb_store(rtdb, 'ao basis', basis)) then
         errmsg = 'bas_rtdb_store failed'
         status = bas_do_destroy(basis)
         return
      endif
      if (.not. bas_add_ucnt_tidy(basis)) then
         errmsg = 'bas_add_ucnt_tidy failed'
         status = bas_do_destroy(basis)
         return
      endif
      if (.not. bas_do_destroy(basis)) then
         errmsg = 'bas_do_destroy failed'
         return
      endif
      ok = .true.
      return
      end

      subroutine nwchemc_z_to_elem(z, elem)
      implicit none
      integer z
      character*16 elem
      character*2 sym(0:36)
      data sym /'X ','H ','He','Li','Be','B ','C ','N ','O ','F ',
     $     'Ne','Na','Mg','Al','Si','P ','S ','Cl','Ar','K ','Ca',
     $     'Sc','Ti','V ','Cr','Mn','Fe','Co','Ni','Cu','Zn','Ga',
     $     'Ge','As','Se','Br','Kr'/
C     Left-align element symbol in character*16 (matches geom_input tags).
      elem = ' '
      if (z .ge. 0 .and. z .le. 36) then
         elem(1:2) = sym(z)
      else
         elem(1:2) = 'X '
      endif
      return
      end

C     Open MPI Fortran bindings use default INTEGER (not necessarily
C     -fdefault-integer-8); force integer*4 for mpi_initialized args.
      subroutine nwchemc_mpi_start_if_needed(owns_mpi)
      implicit none
      integer*4 ierr4, flag4
      integer owns_mpi
      integer tcgready
      external mpi_initialized, pbeginf, tcgready
      owns_mpi = 0
      if (tcgready() .ne. 0) return
      flag4 = 0
      ierr4 = 0
      call mpi_initialized(flag4, ierr4)
      if (flag4 .eq. 0) owns_mpi = 1
      call pbeginf()
      return
      end
