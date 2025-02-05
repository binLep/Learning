﻿// jQuery.autoIMG.js - 2010-04-02 - Tang Bin - http://planeArt.cn/ - MIT Licensed
(function($) {
	// 检测是否支持css2.1 max-width属性
	var isMaxWidth = 'maxWidth' in document.documentElement.style,
	// 检测是否IE7浏览器
	isIE7 = !-[ 1, ] && !('prototype' in Image) && isMaxWidth;

	var maxWidth = 400;

	$.fn.autoIMG = function() {

		return this.find('img').each(function(i, img) {

			// 如果支持max-width属性则使用此，否则使用下面方式
			// if (isMaxWidth) {
			// imgReady(src, function (width, height){
			// img.style.width = width + 'px';
			// img.style.height = height + 'px';
			// });
			//				
			// }else{
			var src = img.src;
			// 隐藏原图
			img.style.display = 'none';
			img.removeAttribute('src');

			// 获取图片头尺寸数据后立即调整图片
			imgReady(src, function(width, height) {
				// 等比例缩小
				if (width > maxWidth) {
					height = maxWidth / width * height, width = maxWidth;
					img.style.width = width + 'px';
					img.style.height = height + 'px';
				} else {
					img.style.width = width + 'px';
					img.style.height = height + 'px';
				}
				// 显示原图
				img.style.display = '';
				img.setAttribute('src', src);
			});
			// }
		});
	};

	// IE7缩放图片会失真，采用私有属性通过三次插值解决
	isIE7&& (function(c, d, s) {
		s = d.createElement('style');
		d.getElementsByTagName('head')[0].appendChild(s);
		s.styleSheet && (s.styleSheet.cssText += c)
				|| s.appendChild(d.createTextNode(c))
	})('img {-ms-interpolation-mode:bicubic}', document);

	/**
	 * 图片头数据加载就绪事件
	 * 
	 * @see http://www.planeart.cn/?p=1121
	 * @param {String}
	 *            图片路径
	 * @param {Function}
	 *            尺寸就绪 (参数1接收width; 参数2接收height)
	 * @param {Function}
	 *            加载完毕 (可选. 参数1接收width; 参数2接收height)
	 * @param {Function}
	 *            加载错误 (可选)
	 */
	var imgReady = (function() {
		var list = [], intervalId = null,

		// 用来执行队列
		tick = function() {
			var i = 0;
			for (; i < list.length; i++) {
				list[i].end ? list.splice(i--, 1) : list[i]();
			}
			;
			!list.length && stop();
		},

		// 停止所有定时器队列
		stop = function() {
			clearInterval(intervalId);
			intervalId = null;
		};

		return function(url, ready, load, error) {
			var check, width, height, newWidth, newHeight, img = new Image();

			img.src = url;

			// 如果图片被缓存，则直接返回缓存数据
			if (img.complete) {
				ready(img.width, img.height);
				load && load(img.width, img.height);
				return;
			}
			;

			// 检测图片大小的改变
			width = img.width;
			height = img.height;
			check = function() {
				newWidth = img.width;
				newHeight = img.height;
				if (newWidth !== width || newHeight !== height ||
				// 如果图片已经在其他地方加载可使用面积检测
				newWidth * newHeight > 1024) {
					ready(newWidth, newHeight);
					check.end = true;
				}
				;
			};
			check();

			// 加载错误后的事件
			img.onerror = function() {
				error && error();
				check.end = true;
				img = img.onload = img.onerror = null;
			};

			// 完全加载完毕的事件
			img.onload = function() {
				load && load(img.width, img.height);
				!check.end && check();
				// IE gif动画会循环执行onload，置空onload即可
				img = img.onload = img.onerror = null;
			};

			// 加入队列中定期执行
			if (!check.end) {
				list.push(check);
				// 无论何时只允许出现一个定时器，减少浏览器性能损耗
				if (intervalId === null)
					intervalId = setInterval(tick, 40);
			}
			;
		};
	})();

})(jQuery);