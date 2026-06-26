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
     $     atmnrs, cell_ang, has_cell, basis_name, theory_name,
     $     scf_type, input_blocks, charge, mult, dft_direct,
     $     dft_smear_on, dft_smear_sigma, dft_smear_spinset,
     $     scf_has_options, scf_maxiter, scf_thresh, scf_tol2e,
     $     driver_has_options, driver_maxiter, driver_tolerance_mode,
     $     driver_gmax_tol, driver_grms_tol, driver_xmax_tol,
     $     driver_xrms_tol, psp_count, psp_elements, psp_types,
     $     psp_names, set_string_count, set_keys, set_values,
     $     typed_set_count, typed_set_keys, typed_set_types,
     $     typed_set_value_counts, typed_set_values,
     $     brillouin_has_options, brillouin_zone_name,
     $     brillouin_monkhorst_pack, brillouin_max_kpoints_print,
     $     energy_h, grad_h_bohr, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "inp.fh"
      integer rtdb, n_atoms, charge, mult, ok
      integer has_cell
      integer psp_count
      integer set_string_count
      integer typed_set_count
      integer brillouin_has_options, brillouin_max_kpoints_print
      integer dft_direct, dft_smear_on, dft_smear_spinset
      integer scf_has_options, scf_maxiter
      integer driver_has_options, driver_maxiter
      integer driver_tolerance_mode
      double precision pos_ang(*)
      double precision cell_ang(*)
      integer atmnrs(*)
      integer psp_types(*)
      integer typed_set_types(*)
      integer typed_set_value_counts(*)
      integer brillouin_monkhorst_pack(3)
      character*64 basis_name
      character*64 theory_name
      character*64 scf_type
      character*64 brillouin_zone_name
      character*4096 input_blocks
      character*16 psp_elements(*)
      character*256 psp_names(*)
      character*128 set_keys(*)
      character*256 set_values(*)
      character*128 typed_set_keys(*)
      character*256 typed_set_values(16,*)
      double precision dft_smear_sigma
      double precision scf_thresh, scf_tol2e
      double precision driver_gmax_tol, driver_grms_tol
      double precision driver_xmax_tol, driver_xrms_tol
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

      call nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, errmsg, status)
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
      call nwchemc_store_brillouin_zone(rtdb, brillouin_has_options,
     $     brillouin_zone_name, brillouin_monkhorst_pack,
     $     brillouin_max_kpoints_print, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_string_sets(rtdb, set_string_count,
     $     set_keys, set_values, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_typed_sets(rtdb, typed_set_count,
     $     typed_set_keys, typed_set_types, typed_set_value_counts,
     $     typed_set_values, errmsg, status)
      if (.not. status) return

      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, scf_has_options, scf_maxiter,
     $     scf_thresh, scf_tol2e, driver_has_options,
     $     driver_maxiter, driver_tolerance_mode, driver_gmax_tol,
     $     driver_grms_tol, driver_xmax_tol, driver_xrms_tol, status)
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

      subroutine nwchem_legacy_dipole(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, basis_name, theory_name,
     $     scf_type, input_blocks, charge, mult, dft_direct,
     $     dft_smear_on, dft_smear_sigma, dft_smear_spinset,
     $     scf_has_options, scf_maxiter, scf_thresh, scf_tol2e,
     $     driver_has_options, driver_maxiter, driver_tolerance_mode,
     $     driver_gmax_tol, driver_grms_tol, driver_xmax_tol,
     $     driver_xrms_tol, psp_count, psp_elements, psp_types,
     $     psp_names, set_string_count, set_keys, set_values,
     $     typed_set_count, typed_set_keys, typed_set_types,
     $     typed_set_value_counts, typed_set_values,
     $     brillouin_has_options, brillouin_zone_name,
     $     brillouin_monkhorst_pack, brillouin_max_kpoints_print,
     $     energy_h, dipole_au, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "inp.fh"
      integer rtdb, n_atoms, charge, mult, ok
      integer has_cell
      integer psp_count
      integer set_string_count
      integer typed_set_count
      integer brillouin_has_options, brillouin_max_kpoints_print
      integer dft_direct, dft_smear_on, dft_smear_spinset
      integer scf_has_options, scf_maxiter
      integer driver_has_options, driver_maxiter
      integer driver_tolerance_mode
      double precision pos_ang(*)
      double precision cell_ang(*)
      integer atmnrs(*)
      integer psp_types(*)
      integer typed_set_types(*)
      integer typed_set_value_counts(*)
      integer brillouin_monkhorst_pack(3)
      character*64 basis_name
      character*64 theory_name
      character*64 scf_type
      character*64 brillouin_zone_name
      character*4096 input_blocks
      character*16 psp_elements(*)
      character*256 psp_names(*)
      character*128 set_keys(*)
      character*256 set_values(*)
      character*128 typed_set_keys(*)
      character*256 typed_set_values(16,*)
      double precision dft_smear_sigma
      double precision scf_thresh, scf_tol2e
      double precision driver_gmax_tol, driver_grms_tol
      double precision driver_xmax_tol, driver_xrms_tol
      double precision energy_h
      double precision dipole_au(*)
      character*(*) errmsg
      integer i
      integer maxat
      parameter (maxat = 64)
      logical status
      external task_energy
      logical task_energy
      external nw_inp_from_character
      logical nw_inp_from_character

      ok = -1
      energy_h = 0.0d0
      do i = 1, 3
         dipole_au(i) = 0.0d0
      enddo
      errmsg = ' '
      if (rtdb .lt. 0 .or. n_atoms .le. 0) then
         errmsg = 'invalid rtdb or n_atoms'
         return
      endif
      if (n_atoms .gt. maxat) then
         errmsg = 'n_atoms exceeds embed maxat=64'
         return
      endif

      call nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, errmsg, status)
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
      call nwchemc_store_brillouin_zone(rtdb, brillouin_has_options,
     $     brillouin_zone_name, brillouin_monkhorst_pack,
     $     brillouin_max_kpoints_print, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_string_sets(rtdb, set_string_count,
     $     set_keys, set_values, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_typed_sets(rtdb, typed_set_count,
     $     typed_set_keys, typed_set_types, typed_set_value_counts,
     $     typed_set_values, errmsg, status)
      if (.not. status) return

      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, scf_has_options, scf_maxiter,
     $     scf_thresh, scf_tol2e, driver_has_options,
     $     driver_maxiter, driver_tolerance_mode, driver_gmax_tol,
     $     driver_grms_tol, driver_xmax_tol, driver_xrms_tol, status)
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

      if (.not. rtdb_get(rtdb, 'task:dipole', mt_dbl, 3,
     $     dipole_au(1))) then
         if (.not. rtdb_get(rtdb, 'scf:dipole', mt_dbl, 3,
     $      dipole_au(1))) then
            if (.not. rtdb_get(rtdb, 'dft:dipole', mt_dbl, 3,
     $         dipole_au(1))) then
               errmsg = 'rtdb_get task/scf/dft dipole failed'
               return
            endif
         endif
      endif

      errmsg = 'ok'
      ok = 0
      return
      end

      subroutine nwchem_legacy_hessian(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, basis_name, theory_name,
     $     scf_type, input_blocks, charge, mult, dft_direct,
     $     dft_smear_on, dft_smear_sigma, dft_smear_spinset,
     $     scf_has_options, scf_maxiter, scf_thresh, scf_tol2e,
     $     driver_has_options, driver_maxiter, driver_tolerance_mode,
     $     driver_gmax_tol, driver_grms_tol, driver_xmax_tol,
     $     driver_xrms_tol, psp_count, psp_elements, psp_types,
     $     psp_names, set_string_count, set_keys, set_values,
     $     typed_set_count, typed_set_keys, typed_set_types,
     $     typed_set_value_counts, typed_set_values,
     $     brillouin_has_options, brillouin_zone_name,
     $     brillouin_monkhorst_pack, brillouin_max_kpoints_print,
     $     hessian_h_bohr2, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "global.fh"
#include "rtdb.fh"
#include "stdio.fh"
#include "errquit.fh"
#include "inp.fh"
      integer rtdb, n_atoms, charge, mult, ok
      integer has_cell
      integer psp_count
      integer set_string_count
      integer typed_set_count
      integer brillouin_has_options, brillouin_max_kpoints_print
      integer dft_direct, dft_smear_on, dft_smear_spinset
      integer scf_has_options, scf_maxiter
      integer driver_has_options, driver_maxiter
      integer driver_tolerance_mode
      double precision pos_ang(*)
      double precision cell_ang(*)
      integer atmnrs(*)
      integer psp_types(*)
      integer typed_set_types(*)
      integer typed_set_value_counts(*)
      integer brillouin_monkhorst_pack(3)
      character*64 basis_name
      character*64 theory_name
      character*64 scf_type
      character*64 brillouin_zone_name
      character*4096 input_blocks
      character*16 psp_elements(*)
      character*256 psp_names(*)
      character*128 set_keys(*)
      character*256 set_values(*)
      character*128 typed_set_keys(*)
      character*256 typed_set_values(16,*)
      double precision dft_smear_sigma
      double precision scf_thresh, scf_tol2e
      double precision driver_gmax_tol, driver_grms_tol
      double precision driver_xmax_tol, driver_xrms_tol
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

      call nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, errmsg, status)
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
      call nwchemc_store_brillouin_zone(rtdb, brillouin_has_options,
     $     brillouin_zone_name, brillouin_monkhorst_pack,
     $     brillouin_max_kpoints_print, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_string_sets(rtdb, set_string_count,
     $     set_keys, set_values, errmsg, status)
      if (.not. status) return
      call nwchemc_apply_direct_typed_sets(rtdb, typed_set_count,
     $     typed_set_keys, typed_set_types, typed_set_value_counts,
     $     typed_set_values, errmsg, status)
      if (.not. status) return

      call nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, scf_has_options, scf_maxiter,
     $     scf_thresh, scf_tol2e, driver_has_options,
     $     driver_maxiter, driver_tolerance_mode, driver_gmax_tol,
     $     driver_grms_tol, driver_xmax_tol, driver_xrms_tol, status)
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

      subroutine nwchemc_apply_direct_typed_sets(rtdb, set_count,
     $     set_keys, set_types, set_value_counts, set_values, errmsg,
     $     ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
#include "inp.fh"
      integer rtdb, set_count
      integer set_types(*)
      integer set_value_counts(*)
      character*128 set_keys(*)
      character*256 set_values(16,*)
      character*(*) errmsg
      logical ok
      integer i, j, l_key, l_value, nvalues, maxsets, maxvalues
      parameter (maxsets = 64)
      parameter (maxvalues = 16)
      integer ivals(maxvalues)
      double precision dvals(maxvalues)
      logical lvals(maxvalues)
      character*256 cvals(maxvalues)

      ok = .false.
      if (set_count .lt. 0 .or. set_count .gt. maxsets) then
         errmsg = 'invalid direct typed set count'
         return
      endif
      do i = 1, set_count
         l_key = inp_strlen(set_keys(i))
         nvalues = set_value_counts(i)
         if (l_key .le. 0 .or. nvalues .le. 0 .or.
     $      nvalues .gt. maxvalues) then
            errmsg = 'invalid direct typed set entry'
            return
         endif
         do j = 1, nvalues
            l_value = inp_strlen(set_values(j,i))
            if (l_value .le. 0) then
               errmsg = 'empty direct typed set value'
               return
            endif
         enddo
         if (set_types(i) .eq. 2) then
            do j = 1, nvalues
               read(set_values(j,i), *, err=900) dvals(j)
            enddo
            call nwchemc_rtdb_put_dbl_array(rtdb, set_keys(i)(1:l_key),
     $         nvalues, dvals, ok)
            if (.not. ok) then
               errmsg = 'direct double set write failed'
               return
            endif
         else if (set_types(i) .eq. 3) then
            do j = 1, nvalues
               read(set_values(j,i), *, err=910) ivals(j)
            enddo
            call nwchemc_rtdb_put_int_array(rtdb, set_keys(i)(1:l_key),
     $         nvalues, ivals, ok)
            if (.not. ok) then
               errmsg = 'direct integer set write failed'
               return
            endif
         else if (set_types(i) .eq. 4) then
            do j = 1, nvalues
               call nwchemc_parse_logical(set_values(j,i),
     $            lvals(j), ok)
               if (.not. ok) then
                  errmsg = 'direct logical set parse failed'
                  return
               endif
            enddo
            call nwchemc_rtdb_put_log_array(rtdb, set_keys(i)(1:l_key),
     $         nvalues, lvals, ok)
            if (.not. ok) then
               errmsg = 'direct logical set write failed'
               return
            endif
         else
            do j = 1, nvalues
               cvals(j) = set_values(j,i)
            enddo
            if (.not. rtdb_cput(rtdb, set_keys(i)(1:l_key),
     $         nvalues, cvals)) then
               errmsg = 'direct string set write failed'
               return
            endif
         endif
      enddo
      ok = .true.
      return
 900  continue
      errmsg = 'direct double set parse failed'
      return
 910  continue
      errmsg = 'direct integer set parse failed'
      return
      end

      subroutine nwchemc_parse_logical(value, parsed, ok)
      implicit none
#include "inp.fh"
      character*(*) value
      logical parsed, ok

      ok = .true.
      if (inp_compare(.false., value, 'true') .or.
     $    inp_compare(.false., value, '.true.') .or.
     $    inp_compare(.false., value, 't')) then
         parsed = .true.
      else if (inp_compare(.false., value, 'false') .or.
     $    inp_compare(.false., value, '.false.') .or.
     $    inp_compare(.false., value, 'f')) then
         parsed = .false.
      else
         ok = .false.
      endif
      return
      end

      subroutine nwchemc_apply_direct_string_sets(rtdb, set_count,
     $     set_keys, set_values, errmsg, ok)
      implicit none
#include "rtdb.fh"
#include "inp.fh"
      integer rtdb, set_count
      character*128 set_keys(*)
      character*256 set_values(*)
      character*(*) errmsg
      logical ok
      integer i, l_key, l_value, maxsets
      parameter (maxsets = 64)

      ok = .false.
      if (set_count .lt. 0 .or. set_count .gt. maxsets) then
         errmsg = 'invalid direct set string count'
         return
      endif
      do i = 1, set_count
         l_key = inp_strlen(set_keys(i))
         l_value = inp_strlen(set_values(i))
         if (l_key .le. 0 .or. l_value .le. 0) then
            errmsg = 'empty direct set string'
            return
         endif
         if (.not. rtdb_cput(rtdb, set_keys(i)(1:l_key), 1,
     $      set_values(i)(1:l_value))) then
            errmsg = 'direct set string write failed'
            return
         endif
      enddo
      ok = .true.
      return
      end

      subroutine nwchemc_apply_method_state(rtdb, theory_name, scf_type,
     $     charge, mult, dft_direct, dft_smear_on, dft_smear_sigma,
     $     dft_smear_spinset, scf_has_options, scf_maxiter,
     $     scf_thresh, scf_tol2e, driver_has_options,
     $     driver_maxiter, driver_tolerance_mode, driver_gmax_tol,
     $     driver_grms_tol, driver_xmax_tol, driver_xrms_tol, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
      integer rtdb, charge, mult
      integer dft_direct, dft_smear_on, dft_smear_spinset
      integer scf_has_options, scf_maxiter
      integer driver_has_options, driver_maxiter
      integer driver_tolerance_mode
      double precision driver_gmax_tol, driver_grms_tol
      double precision driver_xmax_tol, driver_xrms_tol
      character*64 theory_name
      character*64 scf_type
      double precision dft_smear_sigma
      double precision scf_thresh, scf_tol2e
      double precision gmax_tol, grms_tol, xmax_tol, xrms_tol
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
      if (driver_has_options .ne. 0 .and. driver_maxiter .gt. 0) then
         call nwchemc_rtdb_put_int(rtdb, 'driver:nptopt',
     $      driver_maxiter, status)
         ok = ok .and. status
      endif
      if (driver_tolerance_mode .eq. 1) then
         gmax_tol = 0.000015d0
         grms_tol = 0.00001d0
         xmax_tol = 0.00006d0
         xrms_tol = 0.00004d0
      else if (driver_tolerance_mode .eq. 2) then
         gmax_tol = 0.0045d0
         grms_tol = 0.0030d0
         xmax_tol = 0.0180d0
         xrms_tol = 0.0120d0
      endif
      if (driver_tolerance_mode .eq. 1 .or.
     $    driver_tolerance_mode .eq. 2) then
         if (driver_gmax_tol .gt. 0.0d0) gmax_tol = driver_gmax_tol
         if (driver_grms_tol .gt. 0.0d0) grms_tol = driver_grms_tol
         if (driver_xmax_tol .gt. 0.0d0) xmax_tol = driver_xmax_tol
         if (driver_xrms_tol .gt. 0.0d0) xrms_tol = driver_xrms_tol
         call nwchemc_rtdb_put_dbl(rtdb, 'driver:gmax_tol',
     $      gmax_tol, status)
         ok = ok .and. status
         call nwchemc_rtdb_put_dbl(rtdb, 'driver:grms_tol',
     $      grms_tol, status)
         ok = ok .and. status
         call nwchemc_rtdb_put_dbl(rtdb, 'driver:xrms_tol',
     $      xrms_tol, status)
         ok = ok .and. status
         call nwchemc_rtdb_put_dbl(rtdb, 'driver:xmax_tol',
     $      xmax_tol, status)
         ok = ok .and. status
      endif
      if (driver_tolerance_mode .ne. 1 .and.
     $    driver_tolerance_mode .ne. 2) then
         if (driver_gmax_tol .gt. 0.0d0) then
            call nwchemc_rtdb_put_dbl(rtdb, 'driver:gmax_tol',
     $         driver_gmax_tol, status)
            ok = ok .and. status
         endif
         if (driver_grms_tol .gt. 0.0d0) then
            call nwchemc_rtdb_put_dbl(rtdb, 'driver:grms_tol',
     $         driver_grms_tol, status)
            ok = ok .and. status
         endif
         if (driver_xrms_tol .gt. 0.0d0) then
            call nwchemc_rtdb_put_dbl(rtdb, 'driver:xrms_tol',
     $         driver_xrms_tol, status)
            ok = ok .and. status
         endif
         if (driver_xmax_tol .gt. 0.0d0) then
            call nwchemc_rtdb_put_dbl(rtdb, 'driver:xmax_tol',
     $         driver_xmax_tol, status)
            ok = ok .and. status
         endif
      endif
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
         if (scf_has_options .ne. 0) then
            if (scf_maxiter .gt. 0) then
               call nwchemc_rtdb_put_int(rtdb, 'dft:iterations',
     $            scf_maxiter, status)
               ok = ok .and. status
            endif
            if (scf_thresh .gt. 0.0d0) then
               call nwchemc_rtdb_put_dbl(rtdb, 'scf:thresh',
     $            scf_thresh, status)
               ok = ok .and. status
            endif
            if (scf_tol2e .gt. 0.0d0) then
               call nwchemc_rtdb_put_dbl(rtdb, 'dft:tol2e',
     $            scf_tol2e, status)
               ok = ok .and. status
            endif
         endif
      else
         status = rtdb_cput(rtdb, 'scf:input vectors', 1, vectors_in)
         ok = ok .and. status
         status = rtdb_cput(rtdb, 'scf:type', 1, scf_type)
         ok = ok .and. status
         if (scf_has_options .ne. 0) then
            if (scf_maxiter .gt. 0) then
               call nwchemc_rtdb_put_int(rtdb, 'scf:maxiter',
     $            scf_maxiter, status)
               ok = ok .and. status
            endif
            if (scf_thresh .gt. 0.0d0) then
               call nwchemc_rtdb_put_dbl(rtdb, 'scf:thresh',
     $            scf_thresh, status)
               ok = ok .and. status
            endif
            if (scf_tol2e .gt. 0.0d0) then
               call nwchemc_rtdb_put_dbl(rtdb, 'scf:tol2e',
     $            scf_tol2e, status)
               ok = ok .and. status
            endif
         endif
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

      subroutine nwchemc_store_brillouin_zone(rtdb, has_options,
     $     zone_name, monkhorst_pack, max_kpoints_print, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
#include "inp.fh"
      integer rtdb, has_options, max_kpoints_print
      integer monkhorst_pack(3)
      character*64 zone_name
      character*(*) errmsg
      logical ok
      logical status, timereverse
      integer l, nx, ny, nz, nn(3), nks
      integer i1, i2, i3, i, j, k, nks2
      integer l_kvs, k_kvs
      character*255 rtdb_name
      double precision kx, ky, kz, xx, yy, zz
      double precision xxx, yyy, zzz, weight, norm

      ok = .false.
      if (has_options .eq. 0) then
         ok = .true.
         return
      endif
      if (max_kpoints_print .lt. 0) then
         errmsg = 'invalid Brillouin-zone print count'
         return
      endif
      l = inp_strlen(zone_name)
      if (l .le. 0) then
         zone_name = 'zone_default'
         l = 12
      endif
      status = rtdb_cput(rtdb, 'band:zone_name', 1,
     $   zone_name(1:l))
      if (.not. status) then
         errmsg = 'brillouin zone_name write failed'
         return
      endif
      if (max_kpoints_print .gt. 0) then
         rtdb_name = zone_name(1:l)//':number_kvectors_print'
         call nwchemc_rtdb_put_int(rtdb, rtdb_name,
     $      max_kpoints_print, status)
         if (.not. status) then
            errmsg = 'brillouin print count write failed'
            return
         endif
      endif
      if (monkhorst_pack(1) .ne. 0 .and.
     $    monkhorst_pack(2) .ne. 0 .and.
     $    monkhorst_pack(3) .ne. 0) then
         nx = monkhorst_pack(1)
         ny = monkhorst_pack(2)
         nz = monkhorst_pack(3)
         timereverse = .true.
         if (nx .lt. 0) then
            nx = -nx
            timereverse = .false.
         endif
         if (ny .lt. 0) then
            ny = -ny
            timereverse = .false.
         endif
         if (nz .lt. 0) then
            nz = -nz
            timereverse = .false.
         endif
         if (nx .le. 0 .or. ny .le. 0 .or. nz .le. 0) then
            errmsg = 'invalid monkhorst-pack grid'
            return
         endif
         nks = nx * ny * nz
         if (nks .le. 0) then
            errmsg = 'invalid monkhorst-pack k-vector count'
            return
         endif
         if (.not. ma_push_get(mt_dbl, 4 * nks, 'nwchemc_kvs',
     $      l_kvs, k_kvs)) then
            errmsg = 'monkhorst-pack allocation failed'
            return
         endif

         weight = 1.0d0 / dble(nks)
         xxx = 1.0d0 / (2.0d0 * dble(nx))
         yyy = 1.0d0 / (2.0d0 * dble(ny))
         zzz = 1.0d0 / (2.0d0 * dble(nz))
         k = 0
         do i3 = 0, nz - 1
            do i2 = 0, ny - 1
               do i1 = 0, nx - 1
                  xx = 1.0d0 + 2.0d0 * dble(i1) - dble(nx)
                  yy = 1.0d0 + 2.0d0 * dble(i2) - dble(ny)
                  zz = 1.0d0 + 2.0d0 * dble(i3) - dble(nz)
                  dbl_mb(k_kvs + 4 * k) = xx * xxx
                  dbl_mb(k_kvs + 4 * k + 1) = yy * yyy
                  dbl_mb(k_kvs + 4 * k + 2) = zz * zzz
                  dbl_mb(k_kvs + 4 * k + 3) = weight
                  k = k + 1
               enddo
            enddo
         enddo

         if (timereverse) then
            do i = 1, nks - 1
               if (dbl_mb(k_kvs + 4 * (i - 1) + 3) .gt. 0.0d0)
     $            then
                  do j = i + 1, nks
                     kx = dbl_mb(k_kvs + 4 * (i - 1)) +
     $                  dbl_mb(k_kvs + 4 * (j - 1))
                     ky = dbl_mb(k_kvs + 4 * (i - 1) + 1) +
     $                  dbl_mb(k_kvs + 4 * (j - 1) + 1)
                     kz = dbl_mb(k_kvs + 4 * (i - 1) + 2) +
     $                  dbl_mb(k_kvs + 4 * (j - 1) + 2)
                     if (dabs(kx) .lt. 1.0d-9 .and.
     $                   dabs(ky) .lt. 1.0d-9 .and.
     $                   dabs(kz) .lt. 1.0d-9) then
                        dbl_mb(k_kvs + 4 * (i - 1) + 3) =
     $                     dbl_mb(k_kvs + 4 * (i - 1) + 3) +
     $                     dbl_mb(k_kvs + 4 * (j - 1) + 3)
                        dbl_mb(k_kvs + 4 * (j - 1) + 3) = 0.0d0
                     endif
                  enddo
               endif
            enddo
            nks2 = 0
            do i = 1, nks
               if (dbl_mb(k_kvs + 4 * (i - 1) + 3) .gt. 0.0d0)
     $            nks2 = nks2 + 1
            enddo
            i = 1
            do while (i .lt. nks2)
               if (dbl_mb(k_kvs + 4 * (i - 1) + 3) .le. 0.0d0)
     $            then
                  do j = i + 1, nks
                     dbl_mb(k_kvs + 4 * (j - 2)) =
     $                  dbl_mb(k_kvs + 4 * (j - 1))
                     dbl_mb(k_kvs + 4 * (j - 2) + 1) =
     $                  dbl_mb(k_kvs + 4 * (j - 1) + 1)
                     dbl_mb(k_kvs + 4 * (j - 2) + 2) =
     $                  dbl_mb(k_kvs + 4 * (j - 1) + 2)
                     dbl_mb(k_kvs + 4 * (j - 2) + 3) =
     $                  dbl_mb(k_kvs + 4 * (j - 1) + 3)
                  enddo
               else
                  i = i + 1
               endif
            enddo
            do i = 1, nks2
               norm = dbl_mb(k_kvs + 4 * (i - 1)) +
     $            dbl_mb(k_kvs + 4 * (i - 1) + 1) +
     $            dbl_mb(k_kvs + 4 * (i - 1) + 2)
               if (norm .lt. 0.0d0) then
                  dbl_mb(k_kvs + 4 * (i - 1)) =
     $               -dbl_mb(k_kvs + 4 * (i - 1))
                  dbl_mb(k_kvs + 4 * (i - 1) + 1) =
     $               -dbl_mb(k_kvs + 4 * (i - 1) + 1)
                  dbl_mb(k_kvs + 4 * (i - 1) + 2) =
     $               -dbl_mb(k_kvs + 4 * (i - 1) + 2)
               endif
            enddo
            nks = nks2
         endif

         rtdb_name = zone_name(1:l)//':number_kvectors'
         call nwchemc_rtdb_put_int(rtdb, rtdb_name, nks, status)
         if (.not. status) then
            errmsg = 'monkhorst-pack k-vector count write failed'
            status = ma_pop_stack(l_kvs)
            return
         endif
         rtdb_name = zone_name(1:l)//':kvectors'
         call nwchemc_rtdb_put_dbl_array(rtdb, rtdb_name, 4 * nks,
     $      dbl_mb(k_kvs), status)
         if (.not. status) then
            errmsg = 'monkhorst-pack k-vector write failed'
            status = ma_pop_stack(l_kvs)
            return
         endif
         if (timereverse) then
            nn(1) = nx
            nn(2) = ny
            nn(3) = nz
         else
            nn(1) = -nx
            nn(2) = -ny
            nn(3) = -nz
         endif
         rtdb_name = zone_name(1:l)//':monkhorst-pack'
         call nwchemc_rtdb_put_int_array(rtdb, rtdb_name, 3, nn,
     $      status)
         if (.not. status) then
            errmsg = 'monkhorst-pack grid write failed'
            status = ma_pop_stack(l_kvs)
            return
         endif
         status = ma_pop_stack(l_kvs)
         if (.not. status) then
            errmsg = 'monkhorst-pack allocation release failed'
            return
         endif
      endif
      ok = .true.
      return
      end

      subroutine nwchemc_store_geometry(rtdb, n_atoms, pos_ang,
     $     atmnrs, errmsg, ok)
      implicit none
      integer rtdb, n_atoms
      double precision pos_ang(*)
      integer atmnrs(*)
      character*(*) errmsg
      logical ok
      double precision cell_ang(9)
      integer i

      do i = 1, 9
         cell_ang(i) = 0.0d0
      enddo
      call nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, 0, errmsg, ok)
      return
      end

      subroutine nwchemc_store_geometry_cell(rtdb, n_atoms, pos_ang,
     $     atmnrs, cell_ang, has_cell, errmsg, ok)
      implicit none
#include "mafdecls.fh"
#include "rtdb.fh"
#include "stdio.fh"
      integer rtdb, n_atoms
      integer has_cell
      double precision pos_ang(*)
      double precision cell_ang(*)
      integer atmnrs(*)
      character*(*) errmsg
      logical ok
      integer iat, i, j, geom, maxat
      parameter (maxat = 64)
      double precision coords(3,maxat)
      double precision charge(maxat)
      double precision mass(maxat)
      double precision amat(3,3)
      double precision scale
      character*16 tags(maxat)
      logical status
      external geom_create, geom_destroy, geom_set_user_units
      external geom_get_user_scale, geom_cart_set, geom_masses_set
      external geom_rtdb_store, geom_atn_to_default_mass
      external geom_amatrix_set, geom_isystype_set, geom_P1_set
      logical geom_create, geom_destroy, geom_set_user_units
      logical geom_get_user_scale, geom_cart_set, geom_masses_set
      logical geom_rtdb_store, geom_atn_to_default_mass
      logical geom_amatrix_set, geom_isystype_set, geom_P1_set

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
      if (has_cell .ne. 0) then
         do j = 1, 3
            do i = 1, 3
               amat(i,j) = cell_ang(3 * (j - 1) + i) * scale
            enddo
         enddo
         if (.not. geom_amatrix_set(geom, amat)) then
            errmsg = 'geom_amatrix_set failed'
            status = geom_destroy(geom)
            return
         endif
         if (.not. geom_isystype_set(geom, 3)) then
            errmsg = 'geom_isystype_set failed'
            status = geom_destroy(geom)
            return
         endif
         if (.not. geom_P1_set(geom)) then
            errmsg = 'geom_P1_set failed'
            status = geom_destroy(geom)
            return
         endif
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
      character*2 sym(0:118)
      data sym /'X ','H ','He','Li','Be','B ','C ','N ','O ','F ',
     $     'Ne','Na','Mg','Al','Si','P ','S ','Cl','Ar','K ','Ca',
     $     'Sc','Ti','V ','Cr','Mn','Fe','Co','Ni','Cu','Zn','Ga',
     $     'Ge','As','Se','Br','Kr','Rb','Sr','Y ','Zr','Nb','Mo',
     $     'Tc','Ru','Rh','Pd','Ag','Cd','In','Sn','Sb','Te','I ',
     $     'Xe','Cs','Ba','La','Ce','Pr','Nd','Pm','Sm','Eu','Gd',
     $     'Tb','Dy','Ho','Er','Tm','Yb','Lu','Hf','Ta','W ','Re',
     $     'Os','Ir','Pt','Au','Hg','Tl','Pb','Bi','Po','At','Rn',
     $     'Fr','Ra','Ac','Th','Pa','U ','Np','Pu','Am','Cm','Bk',
     $     'Cf','Es','Fm','Md','No','Lr','Rf','Db','Sg','Bh','Hs',
     $     'Mt','Ds','Rg','Cn','Nh','Fl','Mc','Lv','Ts','Og'/
C     Left-align element symbol in character*16 (matches geom_input tags).
      elem = ' '
      if (z .ge. 0 .and. z .le. 118) then
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
