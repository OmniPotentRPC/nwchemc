(require 'ox-publish)

(defun nwchemc-publish-with-pandoc ()
  (let ((base (expand-file-name "orgmode" default-directory))
        (out-root (expand-file-name "source" default-directory)))
    (dolist (org-file (directory-files-recursively base "\\.org\\'"))
      (let* ((rel (file-relative-name org-file base))
             (rst-file
              (expand-file-name
               (concat (file-name-sans-extension rel) ".rst")
               out-root)))
        (make-directory (file-name-directory rst-file) t)
        (unless (= 0 (call-process "pandoc" nil "*nwchemc-pandoc*" t
                                   "-f" "org" "-t" "rst"
                                   org-file "-o" rst-file))
          (with-current-buffer "*nwchemc-pandoc*"
            (princ (buffer-string)))
          (error "pandoc failed for %s" org-file))))))

(if (require 'ox-rst nil t)
    (progn
      (setq org-publish-project-alist
            '(("nwchemc-sphinx-rst"
               :base-directory "./orgmode/"
               :base-extension "org"
               :publishing-directory "./source/"
               :publishing-function org-rst-publish-to-rst
               :recursive t
               :headline-levels 4
               :with-author nil
               :with-toc nil
               :section-numbers nil)))
      (org-publish "nwchemc-sphinx-rst" t))
  (nwchemc-publish-with-pandoc))
